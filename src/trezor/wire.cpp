/*
 * This file is modified from the TREZOR project.
 *
 * Copyright (C) 2017 Ales Katona
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef _WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#include "wire.h"

namespace Trezor {

namespace Wire {

    Device::Device()
    {
        hid_version = 0;
        hid_init();

        hid = NULL;
    }

    Device::~Device() {
        close();
        hid_exit();
    }

    void Device::init()
    {
        close();

        hid = NULL;
        const QString path = getDevicePath();
        if ( path.isEmpty() ) {
            return;
        }

        hid = hid_open_path(path.toStdString().c_str());
        if (!hid) {
            throw wire_error("HID device open failed");
        }
        hid_version = try_hid_version();
        if (hid_version <= 0) {
            throw wire_error("Unknown HID version");
        }
    }

    bool Device::isInitialized() const
    {
        return hid_version > 0;
    }

    const QString Device::getDevicePath()
    {
        QString path;
        hid_device_info* devices = hid_enumerate(0x534c, 0x0001);
        if ( devices != NULL ) {
            hid_device_info* device = devices;
            do {
                // big headache on macos x, the interface is -1 here for some reason so using full path parsing
#ifdef Q_OS_MACX
                if ( QString::fromUtf8(device->path).endsWith("0") ) {
#else
                if ( device->interface_number == 0 ) {
#endif
                     path = QString::fromUtf8(device->path);
                     break;
                }
            } while ( (device = device->next) != NULL );
            hid_free_enumeration(devices);
        }

        return path;
    }

    void Device::close()
    {
        if ( hid != NULL ) {
            hid_close(hid);
        }
        hid = NULL;
    }

    bool Device::isPresent()
    {
        // if we're connected, use try_hid_version to check if connection still works
        if ( hid != NULL ) {
            bool connected = try_hid_version() > 0;
            if ( !connected ) {
                close(); // make sure hid frees resources and we consider ourselves off
            }
            return connected;
        }

        // otherwise enumerate
        return getDevicePath() != NULL;
    }

    // try writing packet that will be discarded to figure out hid version
    int Device::try_hid_version()
    {
        if (!hid) {
            throw wire_error("Try HID version called with null hid handle");
            return 0;
        }
        int r;
        report_type report;

        // try version 2
        report.fill(0xFF);
        report[0] = 0x00;
        report[1] = 0x3F;
        r = hid_write(hid, report.data(), 65);
        if (r == 65) {
            return 2;
        }

        // try version 1
        report.fill(0xFF);
        report[0] = 0x3F;
        r = hid_write(hid, report.data(), 64);
        if (r == 64) {
            return 1;
        }

        // unknown version
        return 0;
    }

    void Device::read_buffered(char_type *data, size_t len)
    {
        if (!hid) {
            throw wire_error("Read called with null hid handle");
        }

        for (;;) {
            if (read_buffer.empty()) {
                buffer_report();
            }
            size_t n = read_report_from_buffer(data, len);
            if (n < len) {
                data += n;
                len -= n;
            } else {
                break;
            }
        }
    }

    void Device::write(char_type const *data, size_t len)
    {
        if (!hid) {
            throw wire_error("Write called with null hid handle");
        }

        for (;;) {
            size_t n = write_report(data, len);
            if (n < len) {
                data += n;
                len -= n;
            } else {
                break;
            }
        }
    }

    size_t Device::read_report_from_buffer(char_type *data, size_t len)
    {
        using namespace std;

        size_t n = min(read_buffer.size(), len);
        auto r1 = read_buffer.begin();
        auto r2 = read_buffer.begin() + n;

        copy(r1, r2, data); // copy to data
        read_buffer.erase(r1, r2); // shift from buffer

        return n;
    }

    void Device::buffer_report()
    {
        if (!hid) {
            throw wire_error("Buffer report called with null hid handle");
        }

        using namespace std;

        report_type report;
        int r;

        do {
            r = hid_read_timeout(hid, report.data(), report.size(), 50);
        } while (r == 0);

        if (r < 0) {
            throw wire_error("HID device read failed");
        }
        if (r > 0) {
            // copy to the buffer, skip the report number
            char_type rn = report[0];
            size_t n = min(static_cast<size_t>(rn),
                              static_cast<size_t>(r - 1));
            copy(report.begin() + 1,
                 report.begin() + 1 + n,
                 back_inserter(read_buffer));
        }
    }

    size_t Device::write_report(char_type const *data, size_t len)
    {
        using namespace std;

        report_type report;
        report.fill(0x00);

        size_t n = min(static_cast<size_t>(63), len);
        size_t report_size = 63 + hid_version;

        switch (hid_version) {
            case 1:
                report[0] = 0x3F;
                copy(data, data + n, report.begin() + 1);
                break;
            case 2:
                report[0] = 0x00;
                report[1] = 0x3F;
                copy(data, data + n, report.begin() + 2);
                break;
        }

        int r = hid_write(hid, report.data(), report_size);
        if (r < 0) {
            throw wire_error{"HID device write failed"};
        }
        if ((size_t)r < report_size) {
            throw wire_error{"HID device write was insufficient"};
        }

        return n;
    }

    void Message::read_from(Device &device)
    {
        Device::char_type buf[6];
        std::uint32_t size;

        device.read_buffered(buf, 1);
        while (buf[0] != '#') {
            device.read_buffered(buf, 1);
        }

        device.read_buffered(buf, 1);
        if (buf[0] != '#') {
            throw header_wire_error{"header bytes are malformed"};
        }

        device.read_buffered(buf, 6);

        id = ntohs((buf[0] << 0) | (buf[1] << 8));
        size = ntohl((buf[2] << 0) | (buf[3] << 8) |
                     (buf[4] << 16) | (buf[5] << 24));

        // 1MB of the message size treshold
        static const std::uint32_t max_size = 1024 * 1024;
        if (size > max_size) {
            throw header_wire_error{"message is too big"};
        }

        data.resize(size);
        device.read_buffered(data.data(), data.size());
    }

    void Message::write_to(Device &device) const
    {
        std::size_t buf_size = 8 + data.size();
        Device::char_type buf[buf_size];

        buf[0] = '#';
        buf[1] = '#';

        std::uint16_t id_ = htons(id);
        buf[2] = (id_ >> 0) & 0xFF;
        buf[3] = (id_ >> 8) & 0xFF;

        std::uint32_t size_ = htonl(data.size());
        buf[4] = (size_ >> 0) & 0xFF;
        buf[5] = (size_ >> 8) & 0xFF;
        buf[6] = (size_ >> 16) & 0xFF;
        buf[7] = (size_ >> 24) & 0xFF;

        std::copy(data.begin(), data.end(), &buf[8]);
        device.write(buf, buf_size);
    }

}

}



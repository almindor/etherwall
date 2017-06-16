#ifndef WIRE_HPP
#define WIRE_HPP

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

#include <hidapi/hidapi.h>
#include <QDebug>
#include <QVariant>
#include <QString>
#include <vector>
#include <array>

namespace Trezor {

namespace Wire {

    class Device
    {
    public:
        typedef std::uint8_t char_type;

        struct wire_error
            : public std::runtime_error
        { using std::runtime_error::runtime_error; };

        Device();
        Device(Device const&) = delete;
        Device &operator=(Device const&) = delete;
        ~Device();

        void init();
        bool isInitialized() const;
        void close();

        bool isPresent();
        // try writing packet that will be discarded to figure out hid version
        int try_hid_version();
        void read_buffered(char_type *data, size_t len);

        void write(char_type const *data, size_t len);
        static const QString getDevicePath();
    private:
        size_t read_report_from_buffer(char_type *data, size_t len);
        void buffer_report();
        size_t write_report(char_type const *data, size_t len);

        typedef std::vector<char_type> buffer_type;
        typedef std::array<char_type, 65> report_type;

        hid_device *hid;
        buffer_type read_buffer;
        int hid_version;
    };

    class Message
    {
    public:
        std::uint16_t id;
        std::vector<std::uint8_t> data;
        QVariant index; // for keeping track on queue side

        typedef Device::wire_error header_wire_error;

        void read_from(Device &device);
        void write_to(Device &device) const;
    };

}

}

#endif // WIRE_HPP

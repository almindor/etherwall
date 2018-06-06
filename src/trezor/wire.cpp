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

#define DEBUG_TRANSFER     0

#define TREZOR1_VID        0x534c
#define TREZOR1_PID        0x0001
#define TREZOR2_VID        0x1209
#define TREZOR2_PID        0x53c1

namespace Trezor {

namespace Wire {

    Device::Device()
    {
        hid_version = 0;
        hid_init();

        hid = NULL;
        
        usb = NULL;
        (void)libusb_init(&usb);
        
        trezor_ver = Trezor_V1;
    }

    Device::~Device() {
        close();
        
        if (usb) {
           libusb_exit(usb);
           usb = NULL;
        } else {
           hid_exit();
        }
    }

    void Device::init()
    {
        close();

        libusb_device_handle* t2 =
           libusb_open_device_with_vid_pid (usb, TREZOR2_VID, TREZOR2_PID);
       
        if (NULL != t2) {           
           if (0 == libusb_claim_interface(t2, 0)) {
              (void)libusb_reset_device(t2);
              
              usb_max_size = libusb_get_max_packet_size(
                 libusb_get_device(t2), 0x81);
            
              if (usb_max_size <= 0) {
                 throw wire_error("failed to get usb packet size.");
              }
                        
              usb_dev = t2;
              trezor_ver = Trezor_V2;
              hid_version = 1;
              return;
           }
        }

        hid = NULL;
        const QString path = getDevicePath();
        if ( path.isEmpty() ) {
            return;
        }

        hid = hid_open_path(path.toStdString().c_str());
        if (!hid) {
#ifdef Q_OS_LINUX
            throw wire_error("HID device open failed, <a href=\"https://doc.satoshilabs.com/trezor-user/settingupchromeonlinux.html\">check your udev permissions</a>");
#else
            throw wire_error("HID device open failed");
#endif
        }
        hid_version = try_hid_version();
        if (hid_version <= 0) {
            throw wire_error("Unknown HID version");
        }
    }

    bool Device::isInitialized() const
    {
        return hid_version > 0 || Trezor_V2 == trezor_ver;
    }

    const QString Device::getDevicePath()
    {  
        QString path;
        hid_device_info* devices = hid_enumerate(TREZOR1_VID, TREZOR1_PID);
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
        
        if (usb_dev) {
           libusb_release_interface(usb_dev, 0);
           libusb_close(usb_dev);
           usb_dev = NULL;
        }
    }

    bool Device::isPresent()
    {
       libusb_device** usb_devices = NULL;
       size_t nDev = libusb_get_device_list(usb, &usb_devices);
       
       if (NULL != usb_devices && nDev > 0) {
          for (size_t i=0; i<nDev; i++) {
             libusb_device* dev = usb_devices[i];
             libusb_device_descriptor desc;
             
             if (0 == libusb_get_device_descriptor(dev, &desc)) {
                if (TREZOR2_VID == desc.idVendor && TREZOR2_PID == desc.idProduct) {
                   return 1;
                }
             }
          }
          
          libusb_free_device_list(usb_devices, 1);
       }
       
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
        if (!hid && !usb_dev) {
            throw wire_error("Read called with null handle");
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
        if (!hid && !usb_dev) {
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
        read_buffer.erase(r1, r2); // shift from buffers

        return n;
    }

#if DEBUG_TRANSFER
    static void dump_hex(const void* data, size_t size) {
    	char ascii[17];
    	size_t i, j;
    	ascii[16] = '\0';

    	for (i = 0; i < size; ++i) {
    		printf("%02X ", ((unsigned char*)data)[i]);
    		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
    			ascii[i % 16] = ((unsigned char*)data)[i];
    		} else {
    			ascii[i % 16] = '.';
    		}
    		if ((i+1) % 8 == 0 || i+1 == size) {
    			printf(" ");
    			if ((i+1) % 16 == 0) {
    				printf("|  %s \n", ascii);
    			} else if (i+1 == size) {
    				ascii[(i+1) % 16] = '\0';
    				if ((i+1) % 16 <= 8) {
    					printf(" ");
    				}
    				for (j = (i+1) % 16; j < 16; ++j) {
    					printf("   ");
    				}
    				printf("|  %s \n", ascii);
    			}
    		}
    	}
    }
#endif

    void Device::buffer_report()
    {
        if (!hid && !usb_dev) {
            throw wire_error("Buffer report called with null hid handle");
        }

        using namespace std;
        
        int r;

        if (usb_dev) {
           std::vector<char_type> data;
           
           data.resize(usb_max_size);
           
           do {
              int recvd = 0;
              
              r = libusb_bulk_transfer(usb_dev, 0x81, data.data(), data.size(), &recvd, 50);
                            
              if (0 == r) {
                 r = recvd;
              } else if (LIBUSB_ERROR_TIMEOUT == r) {
                 r = 0;
              } else {
                 r = -1;
              }
           } while (r == 0);
           
           if (r < 0) {
               throw wire_error("USB device read failed");
           }
           
           if (r > 0) {
               copy(data.begin() + 1 ,
                    data.begin() + data.size(),
                    back_inserter(read_buffer));
           }
#if DEBUG_TRANSFER
           printf("READ %u -----\n", data.size());
           dump_hex(data.data(), data.size());
           printf("----------\n");
#endif
        } else {
           report_type report;
           
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
           
#if DEBUG_TRANSFER
           printf("READ %u -----\n", report.size());
           dump_hex(report.data(), report.size());
           printf("----------\n");
#endif
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

        int r = 0, xferd = 0;

        if (usb_dev) {
           r = libusb_bulk_transfer(usb_dev, 0x1, report.data(), report.size()-1, &xferd, 0);
        } else {
           r = 0;
           xferd = hid_write(hid, report.data(), report_size);
        }
                
        if (xferd < 0 || r != 0) {
            throw wire_error{"HID device write failed"};
        }
        if ((size_t)xferd < report_size) {
            throw wire_error{"HID device write was insufficient"};
        }

#if DEBUG_TRANSFER
        printf("WRITE %u -----\n", report.size());
        dump_hex(report.data(), report.size());
        printf("----------\n");
#endif

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



/*___________________________________________________
 |  _____                       _____ _ _       _    |
 | |  __ \                     |  __ (_) |     | |   |
 | | |__) |__ _ __   __ _ _   _| |__) || | ___ | |_  |
 | |  ___/ _ \ '_ \ / _` | | | |  ___/ | |/ _ \| __| |
 | | |  |  __/ | | | (_| | |_| | |   | | | (_) | |_  |
 | |_|   \___|_| |_|\__, |\__,_|_|   |_|_|\___/ \__| |
 |                   __/ |                           |
 |  GNU/Linux based |___/  Multi-Rotor UAV Autopilot |
 |___________________________________________________|
  
 MAG ADC Calibration Service Implementation

 Copyright (C) 2015 Tobias Simon, Integrated Communication Systems Group, TU Ilmenau

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details. */


#include <msgpack.h>
#include <scl.h>
#include <service.h>

#include "acc_adc_cal.h"


#define SERVICE_NAME "acc_adc_cal"
#define SERVICE_PRIO 99


static void *marg_raw_socket = NULL;
static void *marg_cal_socket = NULL;


SERVICE_MAIN_BEGIN
{
   /* initialize msgpack buffers: */
   msgpack_sbuffer *msgpack_buf = msgpack_sbuffer_new();
   THROW_IF(msgpack_buf == NULL, -ENOMEM);
   msgpack_packer *pk = msgpack_packer_new(msgpack_buf, msgpack_sbuffer_write);
   THROW_IF(pk == NULL, -ENOMEM);
  
   /* initialize SCL: */
   marg_raw_socket = scl_get_socket("acc_raw", "sub");
   THROW_IF(marg_raw_socket == NULL, -EIO);
   marg_cal_socket = scl_get_socket("acc_adc_cal", "pub");
   THROW_IF(marg_cal_socket == NULL, -EIO);

   /* init calibration data: */
   acc_adc_cal_init();
 
   while (1)
   {
      char buffer[1024];
      int ret = scl_recv_static(marg_raw_socket, buffer, sizeof(buffer));
      if (ret > 0)
      {
         msgpack_unpacked msg;
         msgpack_unpacked_init(&msg);
         if (msgpack_unpack_next(&msg, buffer, ret, NULL))
         {
            msgpack_object root = msg.data;
            if (root.type == MSGPACK_OBJECT_ARRAY)
            {
               vec3_t acc;
               vec3_init(&acc);
               FOR_N(i, 3)
                  acc.ve[i] = root.via.array.ptr[i].via.dec;
               acc_adc_cal_apply(&acc);
               msgpack_sbuffer_clear(msgpack_buf);
               msgpack_pack_array(pk, 3);
               PACKFV(acc.ve, 3);
               scl_copy_send_dynamic(marg_cal_socket, msgpack_buf->data, msgpack_buf->size);
            }
         }
         msgpack_unpacked_destroy(&msg);
      }
      else
      {
         msleep(10);
      }
   }
}
SERVICE_MAIN_END

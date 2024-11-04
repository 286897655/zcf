/** 
 * @copyright Copyright © 2020-2024 code by zhaoj
 * 
 * LICENSE
 * 
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

 /**
 * @author zhaoj 286897655@qq.com
 * @brief 
 */

#ifndef ZAV_PROTO_RTSP_H_
#define ZAV_PROTO_RTSP_H_

#include <string>

namespace zav{
    
enum RTSP_TRANSPORT{
    RTSP_TRANSPORT_TCP,// TCP
    RTSP_TRANSPORT_UDP,// UDP
    RTSP_TRANSPORT_UDP_MULTICAST,//MULTICAST
    RTSP_TRANSPORT_HTTP, //HTTP
    RTSP_TRANSPORT_WEBSOCKET,// WEBSOCKET
    RTSP_TRANSPORT_UNKNOWN // UNKNOWN
};

RTSP_TRANSPORT rtsp_transport(const std::string& description);
std::string desc_rtsp_transport(RTSP_TRANSPORT transport);

}//!namespace zav


#endif
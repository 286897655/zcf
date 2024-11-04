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

#include "zav/proto/rtsp.h"

namespace zav{
static constexpr const char* kRTSP_TRANSPORT_DESCRIPTION[6] = {"TCP","UDP","MULTICAST","HTTP","WEBSOCKET","UNKNOWN"};

RTSP_TRANSPORT rtsp_transport(const std::string& description){
    if(description == kRTSP_TRANSPORT_DESCRIPTION[RTSP_TRANSPORT_TCP]){
        return RTSP_TRANSPORT_TCP;
    }else if(description == kRTSP_TRANSPORT_DESCRIPTION[RTSP_TRANSPORT_UDP]){
        return RTSP_TRANSPORT_UDP;
    }else if(description == kRTSP_TRANSPORT_DESCRIPTION[RTSP_TRANSPORT_UDP_MULTICAST]){
        return RTSP_TRANSPORT_UDP_MULTICAST;
    }else if(description == kRTSP_TRANSPORT_DESCRIPTION[RTSP_TRANSPORT_HTTP]){
        return RTSP_TRANSPORT_HTTP;
    }else if(description == kRTSP_TRANSPORT_DESCRIPTION[RTSP_TRANSPORT_WEBSOCKET]){
        return RTSP_TRANSPORT_WEBSOCKET;
    }

    return RTSP_TRANSPORT_UNKNOWN;
}

std::string desc_rtsp_transport(RTSP_TRANSPORT transport){
    return kRTSP_TRANSPORT_DESCRIPTION[transport];
}



};//!namespace zav
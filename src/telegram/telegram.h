#pragma once


#include "AUI/Json/AJson.h"
#include "AUI/Thread/AFuture.h"
#include <AUI/Common/AString.h>
#include <AUI/Common/AOptional.h>
#include <AUI/Image/AImage.h>

namespace telegram {
    struct Message {
        AString text;
        AOptional<AImage> image;
    };
    void postMessage(Message message);


    AFuture<AJson> longPoll();
}

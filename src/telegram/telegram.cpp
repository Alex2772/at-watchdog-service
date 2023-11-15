//
// Created by alex2772 on 9/3/23.
//

#include "telegram.h"
#include "AUI/Json/AJson.h"
#include "AUI/Thread/AFuture.h"
#include "telegram/config.h"
#include <AUI/Util/APreprocessor.h>
#include <AUI/Curl/ACurl.h>
#include <AUI/Thread/AAsyncHolder.h>
#include <AUI/Thread/AComplexFutureOperation.h>
#include <AUI/Curl/AFormMultipart.h>
#include <AUI/Image/png/PngImageLoader.h>
#include <AUI/IO/AStrongByteBufferInputStream.h>

static constexpr auto LOG_TAG = "Telegram";
static constexpr auto TOKEN = AUI_PP_STRINGIZE(TELEGRAM_API_TOKEN);

static AAsyncHolder& telegramAsync() {
    static AAsyncHolder h;
    return h;
}

static AString formatMethodUrl(const char* method) {
    return "https://api.telegram.org/bot{}/{}"_format(TOKEN, method);
}

AFuture<AJson> telegram::longPoll() {
    ACurl::Builder builder(formatMethodUrl("getUpdates"));
    builder.withMethod(ACurl::Method::GET);

    auto operation = _new<AComplexFutureOperation<AJson>>();

    operation << builder.runAsync().onSuccess([operation](ACurl::Response r) {
        operation->supplyResult(AJson::fromBuffer(r.body));
    });

    return operation->makeFuture();
}

void telegram::postMessage(telegram::Message message) {
    ACurl::Builder builder(formatMethodUrl(message.image ? "sendPhoto" : "sendMessage"));
    builder.withMethod(ACurl::Method::POST);

    AFormMultipart params = {
        {"chat_id", {config::CHAT_ID}, },
        {"parse_mode", {"MarkdownV2"}, },
    };

    if (!message.text.empty()) {
        message.text.replaceAll(".", "\\.");
        message.text.replaceAll("-", "\\-");
        message.text.replaceAll("+", "\\+");
        ALogger::debug(LOG_TAG) << "postMessage: " << message.text;
        params[message.image ? "caption" : "text"] = {std::move(message.text)};
    }

    if (message.image) {
        AByteBuffer imageBuffer;
        PngImageLoader::save(imageBuffer, *message.image);
        params["photo"] = {
            .value = _new<AStrongByteBufferInputStream>(std::move(imageBuffer)),
            .filename = "photo.png",
            .mimeType = "image/png",
        };
    }

    //std::cout << AString::fromUtf8(AByteBuffer::fromStream(params.makeInputStream())) << '\n';
    builder.withMultipart(params);

    telegramAsync() << builder.runAsync()
        .onSuccess([](const ACurl::Response& response) {
            if (response.code == ACurl::ResponseCode::HTTP_200_OK) {
                ALOG_DEBUG(LOG_TAG) << "postMessage response " << response.code << " "
                                    << AString::fromUtf8(response.body);
            } else {
                ALogger::err(LOG_TAG) << "postMessage failed " << response.code << " "
                                      << AString::fromUtf8(response.body);
            }
        })
        .onError([](const AException& e) {
            ALogger::err(LOG_TAG) << "postMessage " << e;
        })
        ;
}

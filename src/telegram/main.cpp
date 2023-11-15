#include "AUI/Common/AByteBufferView.h"
#include "AUI/Common/AException.h"
#include "AUI/Common/AObject.h"
#include "AUI/Common/ATimer.h"
#include "AUI/Curl/ACurlMulti.h"
#include "AUI/Json/AJson.h"
#include "AUI/Thread/AAsyncHolder.h"
#include "AUI/Thread/AEventLoop.h"
#include "telegram/config.h"
#include "telegram/telegram.h"
#include <AUI/Platform/Entry.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Util/kAUI.h>

static constexpr auto LOG_TAG = "at-watchdog-service";


class App: public AObject {
public:
    App() {
        mCheckIntervalTimer = _new<ATimer>(config::CHECK_INTERVAL);
        connect(mCheckIntervalTimer->fired, me::onCheckInterval);
        mCheckIntervalTimer->start();
    }

    void run() {
        setupLongPollMessageReceiver();

        AThread::current() * [this] {
            ALogger::info(LOG_TAG) << "Watcher is up and running. Press enter to exit.";
            sendMessage("Сторожевая собака запущена");
        };

        AEventLoop loop;
        IEventLoop::Handle h(&loop);
        loop.loop();
    }
private:
    _<ATimer> mCheckIntervalTimer;
    AAsyncHolder mAsync;

    void setupLongPollMessageReceiver() {
        mAsync << telegram::longPoll().onSuccess([](const AJson& j) {
            ALogger::info(LOG_TAG) << "Update: " << AJson::toString(j);
        });
    }

    void sendMessage(AString message) {
        telegram::postMessage({
            .text = message,
        });
    }

    void reportError(AString message) {
        sendMessage("‼️ {}"_format(message));
    }

    void onCheckInterval() {
        mAsync << ACurl::Builder(config::CHECK_URL).runAsync().onError([&](const AException& r) {
            auto m = "сайт лежит {}: {}"_format(config::CHECK_URL, r.getMessage());
            
            ui_thread {
                reportError(m);
            };
        }).onSuccess([&](ACurl::Response v) {
            auto s = AString::fromUtf8(v.body);
            for (const auto& keyword : config::CHECK_KEYWORDS) {
                if (!s.contains(keyword)) {
                    ui_thread {
                        reportError("сайт выдаёт говно: {}"_format(config::CHECK_URL));
                    };
                    break;
                }
            }
        });
    }
};


AUI_ENTRY {
    auto app = _new<App>();
    app->run();

    return 0;
}
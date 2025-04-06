#include <uv.h>

#include <get-uv-event-loop-napi.h>
#include <iostream>
#include <mdnscpp/LibuvLoop.h>
#include <mdnscpp/Platform.h>
#include <napi.h>

class BrowseContext
{
public:
  BrowseContext(Napi::Env env, const Napi::Function &callback);
  ~BrowseContext();
  void onResultsChanged(std::shared_ptr<mdnscpp::Browser> browser);
  void setHandle(std::shared_ptr<void> handle);

  static void cleanup(const Napi::CallbackInfo &info);
  static void finalizer(Napi::Env env, BrowseContext *ctx);

private:
  Napi::Env env_;
  Napi::AsyncContext asyncCtx_;
  Napi::FunctionReference callbackRef_;
  std::shared_ptr<void> handle_;
};

BrowseContext::BrowseContext(Napi::Env env, const Napi::Function &callback)
    : env_(env), asyncCtx_(env_, "mdns-browse"),
      callbackRef_{Napi::Reference<Napi::Function>::New(callback, 1)}
{
}

BrowseContext::~BrowseContext() {}

void BrowseContext::onResultsChanged(std::shared_ptr<mdnscpp::Browser> browser)
{
  Napi::HandleScope scope(env_);

  callbackRef_.MakeCallback(env_.Global(), {}, asyncCtx_);
}

void BrowseContext::setHandle(std::shared_ptr<void> handle)
{
  handle_ = std::move(handle);
}

void BrowseContext::cleanup(const Napi::CallbackInfo &info)
{
  BrowseContext *ctx = reinterpret_cast<BrowseContext *>(info.Data());
  ctx->handle_ = nullptr;
}
void BrowseContext::finalizer(Napi::Env env, BrowseContext *ctx) { delete ctx; }

class MdnsBrowseAddon : public Napi::Addon<MdnsBrowseAddon>
{
public:
  MdnsBrowseAddon(Napi::Env env, Napi::Object exports)
      : eventLoop_{get_uv_event_loop(env)},
        platform_{mdnscpp::createPlatform(eventLoop_)}
  {
    // In the constructor we declare the functions the add-on makes available
    // to JavaScript.
    DefineAddon(exports,
        {
            InstanceMethod("startBrowse", &MdnsBrowseAddon::startBrowse),
        });
  }

private:
  Napi::Value startBrowse(const Napi::CallbackInfo &info)
  {
    Napi::Env env = info.Env();
    if (info.IsConstructCall())
    {
      Napi::Error::New(env, "Cannot be called as constructor.")
          .ThrowAsJavaScriptException();
      return env.Null();
    }

    if (info.Length() < 2)
    {
      Napi::TypeError::New(env, "Wrong number of arguments.")
          .ThrowAsJavaScriptException();
      return env.Null();
    }

    if (!info[0].IsObject() || !info[1].IsFunction())
    {
      Napi::TypeError::New(env, "Bad argument.").ThrowAsJavaScriptException();
      return env.Null();
    }

    Napi::Object options = info[0].As<Napi::Object>();
    Napi::Function callback = info[1].As<Napi::Function>();

    size_t interfaceIndex = 0;
    std::string type, protocol, domain;
    mdnscpp::IPProtocol ipProtocol = mdnscpp::IPProtocol::Both;

#define GET_OPTION_STR(name, dst)                                              \
  do                                                                           \
  {                                                                            \
    Napi::Value tmp = options.Get(#name);                                      \
    if (!tmp.IsString())                                                       \
    {                                                                          \
      Napi::TypeError::New(env, "Expected options." #name " as string.")       \
          .ThrowAsJavaScriptException();                                       \
      return env.Null();                                                       \
    }                                                                          \
    dst = tmp.As<Napi::String>().Utf8Value();                                  \
  } while (0)

    GET_OPTION_STR(type, type);
    GET_OPTION_STR(protocol, protocol);

    {
      Napi::Value tmp = options.Get("domain");

      if (tmp.IsString())
      {
        domain = tmp.As<Napi::String>().Utf8Value();
      }
    }

    {
      Napi::Value tmp = options.Get("ipProtocol");
      if (tmp.IsString())
      {
        std::string ipProtocolStr = tmp.As<Napi::String>().Utf8Value();

        if (ipProtocolStr == "ipv4")
        {
          ipProtocol = mdnscpp::IPProtocol::IPv4;
        }
        else if (ipProtocolStr == "ipv6")
        {
          ipProtocol = mdnscpp::IPProtocol::IPv6;
        }
        else
        {
          Napi::TypeError::New(env, "Unexpected options.ipProtocol value.")
              .ThrowAsJavaScriptException();
          return env.Null();
        }
      }
    }

    BrowseContext *ctx = new BrowseContext(env, callback);
    ctx->setHandle(platform_->createBrowser(
        type, protocol,
        [ctx](std::shared_ptr<mdnscpp::Browser> browser) {
          ctx->onResultsChanged(browser);
        },
        domain, interfaceIndex, ipProtocol));
    auto cleanup =
        Napi::Function::New<BrowseContext::cleanup>(env, "cleanup", ctx);
    cleanup.AddFinalizer(BrowseContext::finalizer, ctx);
    return cleanup;
  }

  mdnscpp::LibuvLoop eventLoop_;
  std::shared_ptr<mdnscpp::Platform> platform_;
};

// The macro announces that instances of the class `ExampleAddon` will be
// created for each instance of the add-on that must be loaded into Node.js.
NODE_API_ADDON(MdnsBrowseAddon)
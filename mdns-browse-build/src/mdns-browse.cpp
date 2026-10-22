#include <uv.h>

#include "../../src/debug.h"
#include <get-uv-event-loop-napi.h>
#include <iostream>
#include <mdnscpp/LibuvLoop.h>
#include <mdnscpp/Platform.h>
#include <mdnscpp/validation.h>
#include <napi.h>

#include <cmath>
#include <string>

class BrowseContext
{
public:
  BrowseContext(Napi::Env env, const Napi::Function &callback);
  ~BrowseContext();
  void onResultsChanged(std::shared_ptr<mdnscpp::Browser> browser);
  void setHandle(std::shared_ptr<void> handle);

  Napi::Object fromTxtRecord(const mdnscpp::TxtRecord &record);
  Napi::Array fromTxtRecords(const std::vector<mdnscpp::TxtRecord> &records);
  Napi::Object fromBrowseResult(const mdnscpp::BrowseResult &browseResult);
  Napi::Object fromBrowseResult(
      std::shared_ptr<mdnscpp::BrowseResult> browseResult);

  static void cleanup(const Napi::CallbackInfo &info);
  static void finalizer(Napi::Env env, BrowseContext *ctx);

private:
  Napi::Env env_;
  Napi::AsyncContext asyncCtx_;
  Napi::FunctionReference callbackRef_;
  std::shared_ptr<void> handle_;
  std::unordered_map<std::shared_ptr<mdnscpp::BrowseResult>,
      Napi::ObjectReference>
      resultCache_;
};

BrowseContext::BrowseContext(Napi::Env env, const Napi::Function &callback)
    : env_(env), asyncCtx_(env_, "mdns-browse"),
      callbackRef_{Napi::Persistent(callback)}
{
}

BrowseContext::~BrowseContext() { resultCache_.clear(); }

Napi::Object BrowseContext::fromTxtRecord(const mdnscpp::TxtRecord &record)
{
  auto result = Napi::Object::New(env_);
  result.Set("key", Napi::String::New(env_, record.key));
  if (record.value)
    result.Set("value", Napi::String::New(env_, *record.value));
  result.Freeze();
  return result;
}
Napi::Array BrowseContext::fromTxtRecords(
    const std::vector<mdnscpp::TxtRecord> &records)
{
  auto result = Napi::Array::New(env_);
  uint32_t index = 0;
  for (const auto &it : records)
  {
    result.Set(index++, fromTxtRecord(it));
  }
  result.Freeze();
  return result;
}
Napi::Object BrowseContext::fromBrowseResult(
    const mdnscpp::BrowseResult &browseResult)
{
  auto result = Napi::Object::New(env_);
  result.Set("type", Napi::String::New(env_, browseResult.getType()));
  result.Set("protocol", Napi::String::New(env_, browseResult.getProtocol()));
  result.Set("name", Napi::String::New(env_, browseResult.getName()));
  result.Set("domain", Napi::String::New(env_, browseResult.getDomain()));
  result.Set("hostname", Napi::String::New(env_, browseResult.getHostname()));
  result.Set("address", Napi::String::New(env_, browseResult.getAddress()));
  result.Set(
      "interfaceIndex", Napi::Number::New(env_, browseResult.getInterface()));
  result.Set("txtRecords", fromTxtRecords(browseResult.getTxtRecords()));
  result.Set("port", Napi::Number::New(env_, browseResult.getPort()));
  result.Freeze();
  return result;
}

Napi::Object BrowseContext::fromBrowseResult(
    std::shared_ptr<mdnscpp::BrowseResult> browseResult)
{
  auto it = resultCache_.find(browseResult);

  if (it != resultCache_.end())
  {
    return it->second.Value();
  }

  auto result = fromBrowseResult(*browseResult);

  resultCache_.insert({browseResult, Napi::Persistent(result)});

  return result;
}

void BrowseContext::onResultsChanged(std::shared_ptr<mdnscpp::Browser> browser)
{
  Napi::HandleScope scope(env_);
  auto results = Napi::Array::New(env_);
  uint32_t index = 0;

  const auto &browseResults = browser->getResults();

  for (const auto &it : browseResults)
  {
    results.Set(index++, fromBrowseResult(it));
  }

  // Drop all old results from the cache
  std::vector<std::shared_ptr<mdnscpp::BrowseResult>> oldResults;

  for (const auto &it : resultCache_)
  {
    if (browseResults.find(it.first) == browseResults.end())
    {
      oldResults.push_back(it.first);
    }
  }

  for (auto entry : oldResults)
    resultCache_.erase(entry);

  callbackRef_.MakeCallback(env_.Global(), {std::move(results)}, asyncCtx_);
  MDNSCPP_INFO << "returned to onResultsChanged" << MDNSCPP_ENDL;
}

void BrowseContext::setHandle(std::shared_ptr<void> handle)
{
  handle_ = std::move(handle);
}

void BrowseContext::cleanup(const Napi::CallbackInfo &info)
{
  MDNSCPP_INFO << "BrowseContext::cleanup()" << MDNSCPP_ENDL;
  BrowseContext *ctx = reinterpret_cast<BrowseContext *>(info.Data());
  ctx->handle_ = nullptr;
  ctx->resultCache_.clear();
  MDNSCPP_INFO << "BrowseContext::cleanup() done" << MDNSCPP_ENDL;
}
void BrowseContext::finalizer(Napi::Env env, BrowseContext *ctx)
{
  MDNSCPP_INFO << "BrowseContext::finalizer()" << MDNSCPP_ENDL;
  if (ctx->handle_)
  {
    MDNSCPP_INFO << "BrowseContext leaked" << MDNSCPP_ENDL;
  }
  else
  {
    delete ctx;
  }
}

// Throws the error to JavaScript, with a Node.js style `code` property.
static void throwJsError(Napi::Error error, const char *code)
{
  error.Value().Set("code", Napi::String::New(error.Env(), code));
  error.ThrowAsJavaScriptException();
}

// Reads options[name] as a string. undefined and null mean "not set": that is
// an error if `required`, otherwise `dst` is left untouched. Throws a
// JavaScript TypeError and returns false for any other type.
static bool getStringOption(Napi::Env env, const Napi::Object &options,
    const char *name, bool required, std::string &dst)
{
  Napi::Value value = options.Get(name);

  if (value.IsString())
  {
    dst = value.As<Napi::String>().Utf8Value();
    return true;
  }

  if (!required && (value.IsUndefined() || value.IsNull()))
    return true;

  throwJsError(Napi::TypeError::New(env,
                   std::string("Expected options.") + name + " as string."),
      "ERR_INVALID_ARG_TYPE");
  return false;
}

class MdnsBrowseAddon : public Napi::Addon<MdnsBrowseAddon>
{
public:
  MdnsBrowseAddon(Napi::Env env, Napi::Object exports)
      : eventLoop_{get_uv_event_loop(env)}
  {
    // The platform is deliberately not created here. Creating it can fail
    // (e.g. no D-Bus/Avahi daemon), which must not break loading the module.
    // It is created on first use, see getPlatform().
    MDNSCPP_INFO << "MdnsBrowseAddon()" << MDNSCPP_ENDL;
    // In the constructor we declare the functions the add-on makes available
    // to JavaScript.
    DefineAddon(exports,
        {
            InstanceMethod("startBrowse", &MdnsBrowseAddon::startBrowse),
        });
  }

  ~MdnsBrowseAddon() { MDNSCPP_INFO << "~MdnsBrowseAddon()" << MDNSCPP_ENDL; }

private:
  std::shared_ptr<mdnscpp::Platform> getPlatform()
  {
    auto platform = platform_.lock();

    if (!platform)
    {
      platform = mdnscpp::createPlatform(eventLoop_);
      platform_ = platform;
    }

    return platform;
  }

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
      throwJsError(Napi::TypeError::New(env, "Wrong number of arguments."),
          "ERR_MISSING_ARGS");
      return env.Null();
    }

    if (!info[0].IsObject() || !info[1].IsFunction())
    {
      throwJsError(
          Napi::TypeError::New(env, "Bad argument."), "ERR_INVALID_ARG_TYPE");
      return env.Null();
    }

    Napi::Object options = info[0].As<Napi::Object>();
    Napi::Function callback = info[1].As<Napi::Function>();

    size_t interfaceIndex = 0;
    std::string type, protocol, domain;
    mdnscpp::IPProtocol ipProtocol = mdnscpp::IPProtocol::Both;

    if (!getStringOption(env, options, "type", true, type) ||
        !getStringOption(env, options, "protocol", true, protocol) ||
        !getStringOption(env, options, "domain", false, domain))
      return env.Null();

    {
      Napi::Value tmp = options.Get("interfaceIndex");

      // undefined and null mean "not set".
      if (!tmp.IsUndefined() && !tmp.IsNull())
      {
        if (!tmp.IsNumber())
        {
          throwJsError(Napi::TypeError::New(
                           env, "Expected options.interfaceIndex as number."),
              "ERR_INVALID_ARG_TYPE");
          return env.Null();
        }

        const double value = tmp.As<Napi::Number>().DoubleValue();

        // Do not let negative or fractional values wrap or truncate silently.
        // The upper bound is checked by validateBrowseArguments().
        if (!std::isfinite(value) || value < 0 || value != std::floor(value) ||
            value > 9007199254740991.0)
        {
          throwJsError(
              Napi::RangeError::New(env,
                  "options.interfaceIndex must be an integer between 0 and "
                  "2147483647."),
              "ERR_OUT_OF_RANGE");
          return env.Null();
        }

        interfaceIndex = static_cast<size_t>(value);
      }
    }

    {
      Napi::Value tmp = options.Get("ipProtocol");

      if (!tmp.IsUndefined() && !tmp.IsNull())
      {
        if (!tmp.IsString())
        {
          throwJsError(Napi::TypeError::New(
                           env, "Expected options.ipProtocol as string."),
              "ERR_INVALID_ARG_TYPE");
          return env.Null();
        }

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
          throwJsError(
              Napi::TypeError::New(env, "Unexpected options.ipProtocol value."),
              "ERR_INVALID_ARG_VALUE");
          return env.Null();
        }
      }
    }

    // Validate before anything is created, so that invalid arguments never
    // reach the platform.
    if (auto error = mdnscpp::validateBrowseArguments(
            type, protocol, domain, interfaceIndex))
    {
      const std::string message = "Invalid options: " + error->message;

      if (error->argument ==
          mdnscpp::BrowseArgumentError::Argument::InterfaceIndex)
      {
        throwJsError(Napi::RangeError::New(env, message), "ERR_OUT_OF_RANGE");
      }
      else
      {
        throwJsError(
            Napi::TypeError::New(env, message), "ERR_INVALID_ARG_VALUE");
      }
      return env.Null();
    }

    auto platform = getPlatform();
    BrowseContext *ctx = new BrowseContext(env, callback);
    ctx->setHandle(platform->createBrowser(
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
  std::weak_ptr<mdnscpp::Platform> platform_;
};

// The macro announces that instances of the class `MdnsBrowseAddon` will be
// created for each instance of the add-on that must be loaded into Node.js.
NODE_API_ADDON(MdnsBrowseAddon)

#pragma once
#include "SearchEndpoint.g.h"
#include <winrt/Windows.Web.Http.h>
#include <winrt/Windows.Data.Json.h>

namespace winrt::OpenAI::implementation
{
  struct SearchEndpoint : SearchEndpointT<SearchEndpoint>
  {
    SearchEndpoint() {
      wchar_t key[200]{};
      if (GetEnvironmentVariable(L"BING_KEY", key, std::size(key))) {
        ApiKey(key);
      }
    }

    SearchEndpoint(winrt::OpenAI::Engine const& engine) { m_engine = engine; }

    winrt::hstring ApiKey() const noexcept { return m_apiKey; }
    winrt::hstring Name() const noexcept { return L"search"; }
    
    OpenAI::Engine Engine() const noexcept { return m_engine; }
    void Engine(OpenAI::Engine e) { m_engine = e; }

    void ApiKey(hstring const& value) {
      m_apiKey = value;
      SetAuth();
    }
    hstring Market() const noexcept { return m_market; }
    void Market(hstring const& value) { m_market = value; };
    winrt::Windows::Foundation::IAsyncOperation<winrt::OpenAI::Answer> ExecuteAsync(hstring query, hstring originalQuestion);

  private:
    winrt::hstring m_apiKey;
    winrt::hstring m_market{ L"en-US" };
    winrt::Windows::Web::Http::HttpClient m_client;
    void SetAuth();
    std::wstring_view m_endpoint{ L"https://api.bing.microsoft.com" };

    winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Data::Json::JsonObject> SearchAsync(hstring const& query);
    winrt::Windows::Foundation::IAsyncOperation<winrt::hstring> GetTitlesFromSearchResultsAsync(winrt::hstring question, winrt::hstring original);
    winrt::OpenAI::Engine m_engine;

  };
}
namespace winrt::OpenAI::factory_implementation
{
  struct SearchEndpoint : SearchEndpointT<SearchEndpoint, implementation::SearchEndpoint>
  {
  };
}

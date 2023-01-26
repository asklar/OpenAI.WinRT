#pragma once
#include "SearchEndpoint.g.h"
#include <winrt/Windows.Web.Http.h>
namespace winrt::OpenAI::implementation
{
  struct SearchEndpoint : SearchEndpointT<SearchEndpoint>
  {
    SearchEndpoint();

    winrt::hstring ApiKey() const noexcept { return m_apiKey; }
    void ApiKey(hstring const& value) {
      m_apiKey = value;
      SetAuth();
    }
    hstring Market() const noexcept { return m_market; }
    void Market(hstring const& value) { m_market = value; };
    winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Data::Json::JsonObject> ExecuteAsync(hstring const& query);

  private:
    winrt::hstring m_apiKey;
    winrt::hstring m_market{ L"en-US" };
    winrt::Windows::Web::Http::HttpClient m_client;
    void SetAuth();
    std::wstring_view m_endpoint{ L"https://api.bing.microsoft.com" };
  };
}
namespace winrt::OpenAI::factory_implementation
{
  struct SearchEndpoint : SearchEndpointT<SearchEndpoint, implementation::SearchEndpoint>
  {
  };
}

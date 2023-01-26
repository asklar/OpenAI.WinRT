#include "pch.h"
#include "SearchEndpoint.h"
#include "SearchEndpoint.g.cpp"
#include <winrt/Windows.Web.Http.Headers.h>
#include <winrt/Windows.Data.Json.h>

namespace winrt::OpenAI::implementation
{
  using namespace winrt::Windows::Web::Http;
  SearchEndpoint::SearchEndpoint() {

  }
  void SearchEndpoint::SetAuth() {
    m_client.DefaultRequestHeaders().Append(L"Ocp-Apim-Subscription-Key", m_apiKey);
  }
  winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Data::Json::JsonObject> SearchEndpoint::ExecuteAsync(hstring const& query)
  {
    std::wstring uri = std::vformat(L"{}/v7.0/search?q={}&mkt={}", std::make_wformat_args(m_endpoint, query.c_str(), m_market.c_str()));
    auto response = co_await m_client.GetAsync(winrt::Windows::Foundation::Uri{ uri });
    response.EnsureSuccessStatusCode();
    auto str = co_await response.Content().ReadAsStringAsync();
    auto json = winrt::Windows::Data::Json::JsonObject::Parse(str);

    co_return json;
  }
}

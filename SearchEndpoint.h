#pragma once
#include "SearchEndpoint.g.h"
#include "SearchSkill.g.h"
#include <winrt/Windows.Web.Http.h>
#include <winrt/Windows.Data.Json.h>

namespace winrt::OpenAI::implementation
{
  struct SearchSkill : SearchSkillT<SearchSkill>
  {
    SearchSkill(OpenAI::SearchEndpoint e) : m_endpoint(e) {}
    OpenAI::Engine Engine() const noexcept { return m_engine; }
    void Engine(OpenAI::Engine e) { m_engine = e; }
    winrt::OpenAI::Engine m_engine{ nullptr };
    OpenAI::SearchEndpoint SearchEndpoint() const noexcept { return m_endpoint; }
    void SearchEndpoint(OpenAI::SearchEndpoint e) { m_endpoint = e; }
    winrt::Windows::Foundation::IAsyncOperation<winrt::OpenAI::Answer> ExecuteAsync(hstring query, OpenAI::Context context);
    winrt::hstring Name() const noexcept { return L"search"; }
  private:
    OpenAI::SearchEndpoint m_endpoint;
    winrt::Windows::Foundation::IAsyncOperation<winrt::hstring> GetTitlesFromSearchResultsAsync(winrt::hstring question);
  };

  struct SearchEndpoint : SearchEndpointT<SearchEndpoint>
  {
    SearchEndpoint() {
      wchar_t key[200]{};
      if (GetEnvironmentVariable(L"BING_KEY", key, static_cast<DWORD>(std::size(key)))) {
        ApiKey(key);
      }
    }

    winrt::hstring ApiKey() const noexcept { return m_apiKey; }


    void ApiKey(hstring const& value) {
      m_apiKey = value;
      SetAuth();
    }
    hstring Market() const noexcept { return m_market; }
    void Market(hstring const& value) { m_market = value; };
    winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Data::Json::JsonObject> SearchAsync(hstring const& query);

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
  struct SearchEndpoint : SearchEndpointT<SearchEndpoint, implementation::SearchEndpoint> {};
  struct SearchSkill : SearchSkillT<SearchSkill, implementation::SearchSkill> {};
}

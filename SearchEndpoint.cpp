#include "pch.h"
#include "SearchEndpoint.h"
#include "SearchEndpoint.g.cpp"
#include "SearchSkill.g.cpp"

#include <winrt/Windows.Web.Http.Headers.h>

namespace winrt::OpenAI::implementation
{
  using namespace winrt::Windows::Web::Http;
  
  void SearchEndpoint::SetAuth() {
    m_client.DefaultRequestHeaders().Append(L"Ocp-Apim-Subscription-Key", m_apiKey);
  }

  winrt::Windows::Foundation::IAsyncOperation<winrt::hstring> SearchSkill::GetTitlesFromSearchResultsAsync(winrt::hstring question, winrt::hstring original)
  {
    auto searchRes = co_await m_endpoint.SearchAsync(question);
    auto webResults = searchRes.GetNamedObject(L"webPages").GetNamedArray(L"value");
    std::wstring titles;
    for (const auto& v_ : webResults) {
      auto v = v_.GetObject();
      titles += std::wstring{ v.GetNamedString(L"name").c_str() } + L"\n";
    }
    co_return winrt::hstring{ titles };
  }

  winrt::Windows::Foundation::IAsyncOperation<winrt::OpenAI::Answer> SearchSkill::ExecuteAsync(winrt::hstring question, winrt::hstring original)
  {
    auto titles = co_await GetTitlesFromSearchResultsAsync(question, original);
    auto client = m_engine.GetSkill(L"openai");
    auto query = std::vformat(LR"(you are an AI processing the results from a search query in order to answer a question
the results from the search query:
{}

the question: {}

provide your best guess for a possible answer in json form: {{ "answer": "...", "confidence": 0.... }} where the value of confidence is 1 if you are confident in the answer and 0 if not.)", std::make_wformat_args(titles, question));

    
    auto result = co_await client.ExecuteAsync(query, original);
    winrt::Windows::Data::Json::JsonObject resultJson;
    
    auto a = winrt::OpenAI::Answer();
    if (winrt::Windows::Data::Json::JsonObject::TryParse(result.Value(), resultJson)) {
      auto answer = resultJson.GetNamedString(L"answer");
      auto confidence = resultJson.GetNamedNumber(L"confidence");

      a.Value(answer);
      a.Confidence(confidence);
    }

    co_return a;
  }

  winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Data::Json::JsonObject> SearchEndpoint::SearchAsync(hstring const& query)
  {
    std::wstring uri = std::vformat(L"{}/v7.0/search?q={}&mkt={}", std::make_wformat_args(m_endpoint, query.c_str(), m_market.c_str()));
    auto response = co_await m_client.GetAsync(winrt::Windows::Foundation::Uri{ uri });
    response.EnsureSuccessStatusCode();
    auto str = co_await response.Content().ReadAsStringAsync();
    auto json = winrt::Windows::Data::Json::JsonObject::Parse(str);

    co_return json;
  }
}

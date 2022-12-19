#include "pch.h"
#include "OpenAIClient.h"
#include "OpenAIClient.g.cpp"
#include "CompletionRequest.h"
#include <winrt/Windows.Web.Http.h>
#include <winrt/Windows.Web.Http.Headers.h>
#include <winrt/Windows.Data.Json.h>
#include <format>
namespace winrt {
  using namespace Windows::Data::Json;
  using namespace Windows::Foundation;
  using namespace Windows::Foundation::Collections;
  using namespace Windows::Storage::Streams;
  using namespace Windows::Web::Http;
}

namespace winrt::OpenAI::implementation
{
  OpenAIClient::OpenAIClient()
  {
    wchar_t buffer[100]{};
    if (GetEnvironmentVariable(L"OPENAI_KEY", buffer, static_cast<DWORD>(std::size(buffer))) != 0) {
      ApiKey(buffer);
    }
  }

  void OpenAIClient::ApiKey(winrt::hstring v) noexcept
  {
    m_apiKey = v;
    m_client.DefaultRequestHeaders().Authorization(winrt::Headers::HttpCredentialsHeaderValue(L"Bearer", m_apiKey));
  }

  winrt::IAsyncOperation<winrt::IVector<winrt::OpenAI::Choice>> OpenAIClient::GetCompletionAsync(winrt::hstring prompt, winrt::hstring model)
  {
    auto request = winrt::make<CompletionRequest>();
    request.Prompt(prompt);
    request.Model(model);

    return GetCompletionAsync(request);
  }
  winrt::hstring EscapeStringForJson(winrt::hstring v) {
    return winrt::JsonValue::CreateStringValue(v).Stringify();
  }
  winrt::IAsyncOperation<winrt::IVector<winrt::OpenAI::Choice>> OpenAIClient::GetCompletionAsync(winrt::OpenAI::CompletionRequest request)
  {
    std::vector<winrt::OpenAI::Choice> retChoices;
#ifdef _DEBUG
    try
#endif
    {
      auto promptString = EscapeStringForJson(request.Prompt());
      auto modelString = EscapeStringForJson(request.Model());

      constexpr std::wstring_view requestTemplate{ LR"({{
  "model": {},
  "prompt": {},
  "temperature": {},
  "max_tokens": {},
  "n": {},
  "top_p": {}
}})" };
      const std::wstring_view model{ modelString };
      const std::wstring_view prompt{ promptString };
      auto requestJson = std::vformat(requestTemplate, std::make_wformat_args(
        model, prompt, request.Temperature(), request.MaxTokens(), request.NCompletions(), request.TopP()));
      auto content = winrt::HttpStringContent(requestJson, winrt::UnicodeEncoding::Utf8, L"application/json");
      auto response = co_await m_client.PostAsync(CompletionUri(), content);
      auto responseJsonStr = co_await response.Content().ReadAsStringAsync();
      response.EnsureSuccessStatusCode();

      auto responseJson = JsonObject::Parse(responseJsonStr);
      auto choices = responseJson.GetNamedArray(L"choices");
      for (const auto& c : choices) {
        auto retChoice = winrt::make<Choice>();
        const auto& choice = c.GetObject();
        auto retChoiceImpl = winrt::get_self<Choice>(retChoice);
        retChoiceImpl->m_text = choice.GetNamedString(L"text");
        retChoices.push_back(retChoice);
      }
    }
#ifdef _DEBUG      
    catch (std::exception& e) {
      auto x = e.what();
      throw;
    }
    catch (winrt::hresult_error& e) {
      auto x = e.message();
      throw;
    }
#endif
    auto ret = winrt::single_threaded_vector<winrt::OpenAI::Choice>(std::move(retChoices));
    co_return ret;

  }
}

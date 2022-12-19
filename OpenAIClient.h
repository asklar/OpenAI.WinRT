#pragma once

#include "OpenAIClient.g.h"
#include "Choice.g.h"
#include "CompletionRequest.g.h"
#include <winrt/Windows.Web.Http.h>

namespace winrt::OpenAI::implementation
{
    struct Choice : ChoiceT<Choice>
    {
      winrt::hstring Text() const { return m_text; }
      winrt::hstring m_text;
    };

    struct OpenAIClient : OpenAIClientT<OpenAIClient>
    {
        OpenAIClient();
        winrt::hstring ApiKey() const noexcept { return m_apiKey; }
        void ApiKey(winrt::hstring v) noexcept;
        
        winrt::Windows::Foundation::Uri CompletionUri() const noexcept { return m_completionUri; }
        void CompletionUri(winrt::Windows::Foundation::Uri v) noexcept { m_completionUri = v; }
        Windows::Foundation::IAsyncOperation<winrt::Windows::Foundation::Collections::IVector<winrt::OpenAI::Choice>> GetCompletionAsync(winrt::hstring prompt, winrt::hstring model);
        Windows::Foundation::IAsyncOperation<winrt::Windows::Foundation::Collections::IVector<winrt::OpenAI::Choice>> GetCompletionAsync(winrt::OpenAI::CompletionRequest request);
    private:
      winrt::hstring m_apiKey;
      winrt::Windows::Foundation::Uri m_completionUri{ L"https://api.openai.com/v1/completions" };
      winrt::Windows::Web::Http::HttpClient m_client;
    };
}

namespace winrt::OpenAI::factory_implementation
{
    struct OpenAIClient : OpenAIClientT<OpenAIClient, implementation::OpenAIClient>
    {
    };
}

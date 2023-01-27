#pragma once

#include "OpenAIClient.g.h"
#include "Choice.g.h"
#include "CompletionRequest.g.h"
#include "PromptTemplate.g.h"
#include "FewShotTemplate.g.h"
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

        winrt::hstring Name() const noexcept { return L"openai"; }
        
        

        winrt::Windows::Foundation::Uri CompletionUri() const noexcept { return m_completionUri; }
        void CompletionUri(winrt::Windows::Foundation::Uri v) noexcept { m_completionUri = v; }
        Windows::Foundation::IAsyncOperation<winrt::Windows::Foundation::Collections::IVector<winrt::OpenAI::Choice>> GetCompletionAsync(winrt::hstring prompt, winrt::hstring model);
        Windows::Foundation::IAsyncOperation<winrt::Windows::Foundation::Collections::IVector<winrt::OpenAI::Choice>> GetCompletionAsync(winrt::OpenAI::CompletionRequest request);

        winrt::OpenAI::PromptTemplate CreateTemplate(winrt::hstring promptTemplateString);
        winrt::OpenAI::FewShotTemplate CreateFewShotTemplate(winrt::Windows::Foundation::Collections::IVectorView<winrt::hstring> parameters);

        bool UseBearerTokenAuthorization() const noexcept { return m_useBearerTokenAuthorization; }
        void UseBearerTokenAuthorization(bool v) { m_useBearerTokenAuthorization = v; SetAuth(); }

        Windows::Foundation::IAsyncOperation<winrt::OpenAI::Answer> ExecuteAsync(winrt::hstring query, winrt::hstring originalQuery);
    private:
      winrt::hstring m_apiKey;
      winrt::Windows::Foundation::Uri m_completionUri{ L"https://api.openai.com/v1/completions" };
      winrt::Windows::Web::Http::HttpClient m_client;
      bool m_useBearerTokenAuthorization = true;
      
      void SetAuth();
    };
}

namespace winrt::OpenAI::factory_implementation
{
    struct OpenAIClient : OpenAIClientT<OpenAIClient, implementation::OpenAIClient>
    {
    };
}

#pragma once

#include "OpenAIClient.g.h"
#include "Choice.h"
#include "CompletionRequest.g.h"
#include "PromptTemplate.g.h"
#include "FewShotTemplate.g.h"
#include "EmbeddingUtils.g.h"
#include "ChatMessage.g.h"

#include <winrt/Windows.Web.Http.h>

namespace winrt::OpenAI::implementation
{
    struct OpenAIClient : OpenAIClientT<OpenAIClient>
    {
        OpenAIClient();
        winrt::hstring ApiKey() const noexcept { return m_apiKey; }
        void ApiKey(winrt::hstring v) noexcept;
        

        static constexpr std::wstring_view gpt35turboEndpoint = L"https://api.openai.com/v1/chat/completions";

        winrt::Windows::Foundation::Uri CompletionUri() const noexcept { return m_completionUri; }
        void CompletionUri(winrt::Windows::Foundation::Uri v) noexcept { m_completionUri = v; }
        Windows::Foundation::IAsyncOperation<winrt::Windows::Foundation::Collections::IVector<winrt::OpenAI::Choice>> GetCompletionAsync(winrt::hstring prompt, winrt::hstring model);
        Windows::Foundation::IAsyncOperation<winrt::Windows::Foundation::Collections::IVector<winrt::OpenAI::Choice>> GetCompletionAsync(winrt::OpenAI::CompletionRequest request);

        Windows::Foundation::IAsyncOperation<winrt::Windows::Foundation::Collections::IVector<winrt::OpenAI::Choice>> GetChatResponseAsync(winrt::OpenAI::ChatRequest request);
        
        winrt::OpenAI::PromptTemplate CreateTemplate(winrt::hstring promptTemplateString);
        winrt::OpenAI::FewShotTemplate CreateFewShotTemplate(winrt::Windows::Foundation::Collections::IVectorView<winrt::hstring> parameters);

        winrt::Windows::Foundation::Uri EmbeddingUri() const noexcept { return m_embeddingUri; }
        void EmbeddingUri(winrt::Windows::Foundation::Uri v) noexcept { m_embeddingUri = v; }
        Windows::Foundation::IAsyncOperation<winrt::Windows::Foundation::Collections::IVector<double>> GetEmbeddingAsync(winrt::hstring prompt);

        bool UseBearerTokenAuthorization() const noexcept { return m_useBearerTokenAuthorization; }
        void UseBearerTokenAuthorization(bool v) { m_useBearerTokenAuthorization = v; SetAuth(); }


        
    private:
      winrt::hstring m_apiKey;
      winrt::Windows::Foundation::Uri m_completionUri{ L"https://api.openai.com/v1/completions" };
      winrt::Windows::Foundation::Uri m_embeddingUri{ L"https://api.openai.com/v1/embeddings" };
      winrt::Windows::Web::Http::HttpClient m_client;
      bool m_useBearerTokenAuthorization = true;
      void SetAuth();
    };

    struct ChatMessage : ChatMessageT<ChatMessage>
    {
      Property<winrt::OpenAI::ChatRole> Role;
      Property<winrt::hstring> Content;
      
      ChatMessage(winrt::OpenAI::ChatRole role, winrt::hstring content) : Role(role), Content(content) {}
    };

    struct EmbeddingUtils : EmbeddingUtilsT<EmbeddingUtils>
    {
      EmbeddingUtils() = default;

      static double EmbeddingDistance(winrt::Windows::Foundation::Collections::IVectorView<double> const& v1, winrt::Windows::Foundation::Collections::IVectorView<double> const& v2, winrt::OpenAI::Similarity const& similarity);
      static double EmbeddingDistance(winrt::Windows::Foundation::Collections::IVectorView<double> const& v1, winrt::Windows::Foundation::Collections::IVectorView<double> const& v2);
    };

}

namespace winrt::OpenAI::factory_implementation
{
    struct OpenAIClient : OpenAIClientT<OpenAIClient, implementation::OpenAIClient>{};
    struct EmbeddingUtils : EmbeddingUtilsT<EmbeddingUtils, implementation::EmbeddingUtils> {};
    struct ChatMessage : ChatMessageT<ChatMessage, implementation::ChatMessage> {};
}

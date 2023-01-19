#pragma once

#include "PromptTemplate.g.h"
#include "FewShotTemplate.g.h"
#include "OpenAIClient.h"
namespace winrt::OpenAI::implementation
{
  struct TemplateBase {
    winrt::OpenAI::OpenAIClient m_client;
    
  };

    struct PromptTemplate : PromptTemplateT<PromptTemplate>, TemplateBase
    {
        PromptTemplate() = default;

        winrt::hstring Template() const { return m_template; }
        void Template(winrt::hstring value) { m_template = value; }

        Windows::Foundation::IAsyncOperation<winrt::hstring> FormatAsync(winrt::Windows::Foundation::Collections::IMapView<winrt::hstring, winrt::hstring> parameters);
        
        winrt::hstring m_template;
    };

    struct FewShotTemplate : FewShotTemplateT<FewShotTemplate>, TemplateBase
    {

      std::vector<std::wstring> m_parameters;

      winrt::Windows::Foundation::Collections::IVector<winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::hstring>> Examples() {
        return m_examples;
      }
      void Examples(winrt::Windows::Foundation::Collections::IVector<winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::hstring>> value) { m_examples = value; }
      
      winrt::Windows::Foundation::Collections::IVector<winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::hstring>> m_examples;

      winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Foundation::Collections::IMapView<winrt::hstring, winrt::hstring> > ExecuteAsync(winrt::hstring input);
    };
}

namespace winrt::OpenAI::factory_implementation
{
    struct PromptTemplate : PromptTemplateT<PromptTemplate, implementation::PromptTemplate>
    {
    };

    struct FewShotTemplate : FewShotTemplateT<FewShotTemplate, implementation::FewShotTemplate>{};
}

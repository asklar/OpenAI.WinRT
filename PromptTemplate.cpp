#include "pch.h"
#include "PromptTemplate.h"
#if __has_include("PromptTemplate.g.cpp")
#include "PromptTemplate.g.cpp"
#endif
#if __has_include("FewShotTemplate.g.cpp")
#include "FewShotTemplate.g.cpp"
#endif
#include <format>
#include <numeric>
namespace winrt::OpenAI::implementation
{
  template<typename TParam>
  std::wstring ProcessTemplate(std::wstring_view templateStr, const TParam& parameters)
  {
    std::wstring str{ templateStr };
    for (const auto& [k, v] : parameters) {
      std::wstring before = L"{" + std::wstring{ k } + L"}";
      std::wstring::size_type pos = 0;
      while ((pos = str.find(before)) != std::wstring::npos) {
        str.replace(pos, before.length(), v);
        pos += v.size();
      }
    }
    return str;
  }

  Windows::Foundation::IAsyncOperation<winrt::hstring> PromptTemplate::FormatAsync(winrt::Windows::Foundation::Collections::IMapView<winrt::hstring, winrt::hstring> parameters) {
    winrt::hstring prompt{ ProcessTemplate(m_template.c_str(), parameters) };
    CompletionRequest cr;
    cr.MaxTokens(4000);
    cr.Prompt(prompt);
    cr.Model(L"text-davinci-003");
    auto completion = co_await m_client.GetCompletionAsync(cr);
    auto first = completion.GetAt(0);
    auto text = first.Text();
    co_return text;
  }

  auto ParseResponse(std::wstring_view text) {
    std::unordered_map<winrt::hstring, winrt::hstring> response;
    std::wstring_view::size_type pos = 0;
    while (pos != std::wstring_view::npos) {
      auto colon = text.find(':', pos);
      if (colon != std::wstring_view::npos) {
        auto key = text.substr(pos, colon - pos);
        auto newline = text.find('\n', colon);
        auto value = text.substr(colon + 1 + 1 /* skip space */, newline - colon - 2);
        response.emplace(key, value);
        if (newline != std::wstring_view::npos) {
          pos = newline + 1;
        } else break;
      }
    }
    return response;
  }

  winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Foundation::Collections::IMapView<winrt::hstring, winrt::hstring> > FewShotTemplate::ExecuteAsync(winrt::hstring input)
  {
    std::wstring _template;
    for (const auto& p : m_parameters) {
      _template += p + L": {" + p + L"}\n";
    }
    _template += L"\n";

    auto prompt = std::accumulate(m_examples.begin(), m_examples.end(), std::wstring{}, [this, _template](const auto& a, const auto& b) {
      auto oneExample = ProcessTemplate(_template, b);
      return a + L"\n" + oneExample;
    });

    prompt += L"\n";
    prompt += m_parameters[0] + L": " + input.begin() + L"\n";

    CompletionRequest cr;
    cr.MaxTokens(4000);
    cr.Prompt(prompt);
    cr.Model(L"text-davinci-003");
    auto completion = co_await m_client.GetCompletionAsync(cr);
    auto first = completion.GetAt(0);
    auto text = first.Text();

    auto response = ParseResponse(text);
    
    auto ret = winrt::single_threaded_map<hstring, hstring>(std::move(response));
    co_return ret.GetView();
  
  }
/*
  Windows::Foundation::IAsyncOperation<winrt::hstring> FewShotTemplate::FewShotAsync(winrt::Windows::Foundation::Collections::IVectorView<winrt::Windows::Foundation::Collections::IMapView<winrt::hstring, winrt::hstring>> parameters)
  {
    auto prompt = std::accumulate(parameters.begin(), parameters.end(), std::wstring{}, [this](const auto& a, const auto& b) {
      auto prompt = ProcessTemplate(m_template.c_str(), b);
    return a + L"\n" + prompt;
      });

    co_return winrt::hstring{ prompt };
  }
  */
}

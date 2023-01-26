#include "pch.h"
#include "Engine.h"
#if __has_include("Engine.g.cpp")
#include "Engine.g.cpp"
#endif

#include <winrt/Windows.Data.Json.h>
#include <set>

#include <Calculator.h>

using namespace winrt::Windows::Data::Json;

namespace winrt::OpenAI::implementation
{
  winrt::Windows::Foundation::IAsyncOperation<winrt::hstring> Engine::GetTitlesFromSearchResultsAsync(winrt::hstring question)
  {
    auto searchRes = co_await m_search.ExecuteAsync(question);
    auto webResults = searchRes.GetNamedObject(L"webPages").GetNamedArray(L"value");
    std::wstring titles;
    for (const auto& v_ : webResults) {
      auto v = v_.GetObject();
      titles += std::wstring{ v.GetNamedString(L"name").c_str() } + L"\n";
    }
    co_return winrt::hstring{ titles };
  }

  winrt::Windows::Foundation::IAsyncOperation<winrt::OpenAI::Answer> Engine::AnswerFromSearchFlowAsync(winrt::hstring question)
  {
    auto titles = co_await GetTitlesFromSearchResultsAsync(question);

    auto query = std::vformat(LR"(you are an AI processing the results from a search query in order to answer a question
the results from the search query:
{}

the question: {}

provide your best guess for a possible answer in json form: {{ "answer": "...", "confidence": 0.... }} where the value of confidence is 1 if you are confident in the answer and 0 if not.)", std::make_wformat_args(titles, question));

    auto cr = winrt::OpenAI::CompletionRequest{};
    cr.Prompt({ query });
    cr.NCompletions(1);
    cr.MaxTokens(2000);
    auto resultCompletion = co_await m_client.GetCompletionAsync(cr);
    winrt::Windows::Data::Json::JsonObject resultJson;
    std::wstring result{ resultCompletion.GetAt(0).Text() };

    auto a = winrt::make<Answer>();
    if (winrt::Windows::Data::Json::JsonObject::TryParse(result, resultJson)) {
      auto answer = resultJson.GetNamedString(L"answer");
      auto confidence = resultJson.GetNamedNumber(L"confidence");

      a.Value(answer);
      a.Confidence(confidence);
    }

    co_return a;
  }
  winrt::Windows::Foundation::IAsyncOperation<winrt::OpenAI::Answer> Engine::CalculatorAsync(winrt::hstring expression) {
    auto a = winrt::make<Answer>();

    try {
      auto x = calculator::eval<double>(winrt::to_string(expression));
      a.Value(winrt::to_hstring(x));
      a.Confidence(1);
      co_return a;
    }
    catch (...) {}
    auto cr = winrt::OpenAI::CompletionRequest{};
    cr.MaxTokens(2000);
    cr.Prompt(std::vformat(LR"(You are an AI designed to calculate arithmetic expressions. Reply with the simplest answer in the form of a json: {{ "answer": "...", "confidence": ... }}.
The confidence is a number between 0 and 1 about how confident you are in your answer.
The expression is: {}
)", std::make_wformat_args(expression.c_str())));

    auto completion = co_await m_client.GetCompletionAsync(cr);
    auto completion1st = completion.GetAt(0).Text();
    JsonObject completionJson;
    if (JsonObject::TryParse(completion1st, completionJson)) {
      if (completionJson.HasKey(L"answer")) {
        a.Value(completionJson.GetNamedString(L"answer"));
        a.Confidence(completionJson.GetNamedNumber(L"confidence"));
      }
    }
    co_return a;
  }

  winrt::Windows::Foundation::IAsyncOperation<winrt::OpenAI::Answer> Engine::AskAsync(winrt::hstring question)
  {
    std::wstring history;

    auto round = 0u;
    std::set<std::wstring> questions;
    while (round < m_maxSteps || m_maxSteps == -1) {
      auto cr = winrt::OpenAI::CompletionRequest{};
      auto mainQuery = std::vformat(LR"(You are in a 3-way conversation with the user and a REPL. Your task is to drive the REPL in order to answer the user's question. The REPL provides tools to get more information to answer the user questions accurately. 
The REPL tools are:
{{ [ 
  {{ "tool": "calculator", "description": "calculates arithmetic expressions" }}, 
  {{ "tool": "search", "description": "searches the web and returns the title of the first result" }}
] }}

If you can answer the question, do so in json form as {{ "answer": "...", "confidence": ... }}). The confidence is a number between 0 and 1 about how confident you are in your answer.
If you cannot answer the question and need more information, reply with a json with a call to an appropriate REPL tool to obtain more information and nothing more.  Only one call to a tool should be specified.
The format of the json for calling a REPL tool is {{"tool": "...", "value": "..."}}
The calculator REPL tool accepts the following operators: +,-,*,/,** and only operates on numbers.
To call the calculator tool to calculate 23+42 you would reply with {{"tool": "calculator", "value":"23+42"}}
To call the search tool to search for Joe Biden you would reply with {{"tool": "search", "value":"Joe Biden"}}
Each call to a tool should be for the simplest question possible. Break down bigger questions into smaller ones and only return the first (smallest) question.

Here's the user question: {}

{}

Don't provide an explanation, only return the json.
)", std::make_wformat_args(question, history));
      cr.Prompt(mainQuery);
      cr.MaxTokens(2000);
      cr.NCompletions(1);
      if (m_send && round == 0) m_send(*this, winrt::OpenAI::EngineStepEventArgs{ round++, L"OpenAI", winrt::hstring{ question } });
      auto completion = co_await m_client.GetCompletionAsync(cr);
      auto completion1st = completion.GetAt(0).Text();
      JsonObject completionJson;
      if (JsonObject::TryParse(completion1st, completionJson)) {
        if (completionJson.HasKey(L"answer")) {
          auto a = winrt::make<Answer>();
          a.Value(completionJson.GetNamedString(L"answer"));
          a.Confidence(completionJson.GetNamedNumber(L"confidence"));
          if (m_receive) { m_receive(*this, winrt::OpenAI::EngineStepEventArgs{ round, L"OpenAI", a.Value() }); }
          co_return a;
          break;
        } else if (completionJson.HasKey(L"tool")) {
          auto tool = completionJson.GetNamedString(L"tool");
          auto value = completionJson.GetNamedString(L"value");
          if (m_send) { m_send(*this, winrt::OpenAI::EngineStepEventArgs{ round, tool, value }); }
          winrt::OpenAI::Answer bestResult{ nullptr };

          if (tool == L"search") {
            if (questions.find(value.c_str()) == questions.end()) {
              bestResult = co_await AnswerFromSearchFlowAsync(value);
              history += completion1st + L"\n";
              history += bestResult.Value() + L"\n";
            } else {
              throw std::exception("we've already seen this question");
            }
          } else if (tool == L"calculator") {
            bestResult = co_await CalculatorAsync(value);
            history += completion1st + L"\n";
            history += bestResult.Value() + L"\n";
          }

          if (m_receive) { m_receive(*this, winrt::OpenAI::EngineStepEventArgs{ round, tool, bestResult.Value() }); }

        }
        round++;
      } else {
        throw winrt::hresult_invalid_argument();
      }
    }


  }
}

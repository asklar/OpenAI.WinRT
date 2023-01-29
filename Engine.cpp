#include "pch.h"
#include "Engine.h"
#if __has_include("Engine.g.cpp")
#include "Engine.g.cpp"
#endif
#include "Answer.g.cpp"
#include "Skill.g.cpp"
#include "Context.g.cpp"

#include <winrt/Windows.Data.Json.h>
#include <set>
#include <numeric>

#include <Calculator.h>

using namespace winrt::Windows::Data::Json;

namespace winrt::OpenAI::implementation
{


  Engine::Engine() {
    m_skills = winrt::single_threaded_vector<ISkill>({});
  }

  auto StringFromJson(JsonObject o, hstring key)
  {
    std::wstring value;
    auto v = o.Lookup(key);

    switch (v.ValueType()) {
    case JsonValueType::String:
      value = v.GetString(); break;
    case JsonValueType::Array:
    {
      auto arr = v.GetArray();
      value = std::accumulate(arr.begin(), arr.end(), std::wstring{ L"[" }, [](const auto& i, const auto& v) { return i + L",\"" + v.GetString() + L"\""; }).erase(1, 1) + L"]";
      break;
    }

    }
    return value;
  }

  auto CreateStepArgs(OpenAI::Context context, std::wstring_view toolName, std::wstring_view question)
  {
    auto stepArgs = winrt::make<EngineStepEventArgs>();
    auto sa = winrt::get_self<EngineStepEventArgs>(stepArgs);
    sa->m_context = context;
    sa->m_endpointName = winrt::hstring{ toolName };
    sa->m_value = winrt::hstring{ question };
    return stepArgs;
  }

  winrt::Windows::Foundation::IAsyncOperation<winrt::OpenAI::Answer> Engine::AskAsync(winrt::hstring question)
  {
    std::wstring history;
    auto context_ = winrt::make<Context>();
    auto context = winrt::get_self<Context>(context_);
    
    std::set<std::wstring> questions;

    auto skillTemplate = LR"({{ "tool": "{}" }})";
    std::wstring skillsJson;
    for (const auto& skill : m_skills) {
      if (skill.Name() == L"openai") continue;
      auto snippet = std::vformat(skillTemplate, std::make_wformat_args(skill.Name()));
      if (skillsJson != L"") skillsJson += L", ";
      skillsJson += snippet;
    }

    while (context->m_step < m_maxSteps || m_maxSteps == -1) {
      auto cr = winrt::OpenAI::CompletionRequest{};
      auto mainQuery = std::vformat(LR"(You are in a 3-way conversation with the user and a REPL. Your task is to drive the REPL in order to answer the user's question. The REPL provides tools to get more information to answer the user questions accurately. 
The REPL tools are:
{{ [ {} ] }}

If you can answer the question, do so in json form as {{ "answer": "...", "confidence": ... }}). The confidence is a number between 0 and 1 about how confident you are in your answer.
If you cannot answer the question and need more information, reply with a json with a call to an appropriate REPL tool to obtain more information and nothing more.  Only one call to a tool should be specified.
The format of the json for calling a REPL tool is {{"tool": "...", "value": "..."}}
Do not call more than one tool at a time.
The calculator REPL tool accepts the following operators: +,-,*,/,** and only operates on numbers.
To call the calculator tool to calculate 23+42 you would reply with {{"tool": "calculator", "value":"23+42"}}
To call the search tool to search for Joe Biden you would reply with {{"tool": "search", "value":"Joe Biden"}}
Each call to a tool should be for the simplest question possible. Break down bigger questions into smaller ones and only return the first (smallest) question.

Here's the user question: {}

{}

Don't provide an explanation, only return the json.
)", std::make_wformat_args(skillsJson, question, history));
      
      auto stepArgs = CreateStepArgs(context_, L"OpenAI", question);
      if (m_send && context->m_step == 0) m_send(*this, stepArgs);
      auto completion = co_await Client().ExecuteAsync(mainQuery, context_);
      
      JsonObject completionJson;
      if (JsonObject::TryParse(completion.Value(), completionJson)) {
        if (completionJson.HasKey(L"answer")) {
          auto a = winrt::make<Answer>();
          auto answer = StringFromJson(completionJson, L"answer");
          a.Value(answer);
          a.Confidence(completionJson.GetNamedNumber(L"confidence"));
          if (m_receive) { m_receive(*this, CreateStepArgs(context_, L"OpenAI", a.Value())); }
          co_return a;
          break;
        } else if (completionJson.HasKey(L"tool")) {
          auto tool = completionJson.GetNamedString(L"tool");
          winrt::hstring value{ StringFromJson(completionJson, L"value") };
          
          //} = completionJson.GetNamedString(L"value");
          if (m_send) { m_send(*this, CreateStepArgs(context_, tool, value)); }
          winrt::OpenAI::Answer bestResult{ nullptr };

          // try {
            auto skill = GetSkill(tool);
            bestResult = co_await skill.ExecuteAsync(value, context_);
            history += completion.Value() + L"\n";
            history += bestResult.Value() + L"\n";
          //} catch (...){}

            if (m_receive) { m_receive(*this, CreateStepArgs(context_, tool, bestResult.Value())); }

        }
        context->m_step++;
      } else {
        throw winrt::hresult_invalid_argument();
      }
    }


  }
}

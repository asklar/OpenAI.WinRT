// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/openai.h>
#include <winrt/builders/OpenAI.CompletionRequest.h>
#include <winrt/builders/OpenAI.OpenAIClient.h>
#include <winrt/builders/OpenAI.SearchEndpoint.h>
#include <winrt/builders/OpenAI.Engine.h>
#include <winrt/Windows.Data.Json.h>
#include <numeric>
#include <format>

#include "../Calculator.h"

using namespace winrt;
using namespace Windows::Data::Json;

winrt::Windows::Foundation::IAsyncOperation<winrt::OpenAI::Answer> CalculatorAsync(winrt::hstring expression, winrt::hstring original, OpenAI::Engine engine) {
  auto a = OpenAI::Answer{};

  try {
    auto x = calculator::eval<double>(winrt::to_string(expression));
    a.Value(winrt::to_hstring(x));
    a.Confidence(1);
    co_return a;
  }
  catch (...) {}

  auto prompt = std::vformat(LR"(You are an AI designed to calculate arithmetic expressions. Reply with the simplest answer in the form of a json: {{ "answer": "...", "confidence": ... }}.
The confidence is a number between 0 and 1 about how confident you are in your answer.
The expression is: {}
)", std::make_wformat_args(expression.c_str()));


  auto client = engine.GetSkill(L"openai");
  auto completion = co_await client.ExecuteAsync(prompt, original);
  auto completion1 = completion.Value();
  JsonObject completionJson;
  if (JsonObject::TryParse(completion1, completionJson)) {
    if (completionJson.HasKey(L"answer")) {
      a.Value(completionJson.GetNamedString(L"answer"));
      a.Confidence(completionJson.GetNamedNumber(L"confidence"));
    }
  }
  co_return a;
}


auto Sort(const winrt::hstring& expression, const winrt::hstring& original, const OpenAI::Engine& engine)->winrt::Windows::Foundation::IAsyncOperation<winrt::OpenAI::Answer> {
  std::vector<std::wstring> items;

  JsonArray arr;
  if (JsonArray::TryParse(expression, arr)) {
    for (const auto& a : arr) items.push_back(a.GetString().c_str());
  } else {
    std::wstringstream ss(expression.c_str());
    std::wstring item;
    while (std::getline(ss, item, L',')) {
      items.push_back(item.substr(item.find_first_not_of(L' ')));
    }
  }
  std::sort(items.begin(), items.end());
  std::wstring out = std::accumulate(items.begin(), items.end(), std::wstring{ L"[" }, [](const auto& i, const auto& v) { return i + L",\"" + v  + L"\""; }).erase(1, 1) + L"]";
  auto answer = OpenAI::Answer(winrt::hstring{ out.c_str() });
  co_return answer;

}

int main()
{
  winrt::init_apartment(/*winrt::apartment_type::multi_threaded*/);

  auto searchEndpoint = winrt::OpenAI::builders::SearchEndpoint();

  auto openaiEndpoint = winrt::OpenAI::builders::OpenAIClient()
    .CompletionUri(winrt::Windows::Foundation::Uri{ L"https://lrsopenai.openai.azure.com/openai/deployments/Text-Davinci3-Deployment/completions?api-version=2022-12-01" })
    .UseBearerTokenAuthorization(false)
    ;

  auto calculator = winrt::OpenAI::Skill(L"calculator", OpenAI::SkillHandlerAsync{ &CalculatorAsync });

  auto sort = winrt::OpenAI::Skill(L"sortListAlphabetical", { &Sort });

  auto files = winrt::OpenAI::Skill(L"files", [](winrt::hstring expression, winrt::hstring original, OpenAI::Engine engine) -> winrt::Windows::Foundation::IAsyncOperation<winrt::OpenAI::Answer> {
    auto client = engine.GetSkill(L"openai");
    auto intent = co_await client.ExecuteAsync(std::vformat(LR"(You are an assistant helping the user with their files on Windows. 

Respond with a json that contains the folder and the set of files to fetch. Use * for wildcards.
For example: {{ "folder": "documents", "filespec": "*" }} to fetch all files from the documents folder.
or {{ "folder": "pictures", "filespec": "*.png" }} to fetch all png files.

Here are the files the user wants: {}

)", std::make_wformat_args(expression)), original);
    auto text = intent.Value();
    auto json = JsonObject::Parse(text);
    std::wcout << "[files skill] " << text << "\n";
    co_return OpenAI::Answer(LR"(hammerthrow.txt
taylorSwiftTopHits.docx
AgneHammerThrowRecord.md
OpenAIMonetizationPlan.pptx
ASklarIndieMovie.mp4
)");
  });

  auto engine = winrt::OpenAI::builders::Engine{}
    .Skills({ 
      OpenAI::SearchSkill(searchEndpoint),
      OpenAI::GPTSkill(openaiEndpoint),
      calculator, 
      files, 
      sort,
  });
  engine.ConnectSkills();



  // For debugging purposes:
  engine.EngineStepSend([](const auto& engine, const winrt::OpenAI::EngineStepEventArgs& args) {
        std::wcout << L"Step " << args.StepNumber << L" [" << args.EndpointName << L"]  --> " << args.Value << L"\n";
      });
  engine.EngineStepReceive([](const auto& engine, const winrt::OpenAI::EngineStepEventArgs& args) {
    std::wcout << L"Step " << args.StepNumber << L" [" << args.EndpointName << L"]  <-- " << args.Value << L"\n";
    });


  auto answer = engine.AskAsync({ L"get the files on my desktop folder and sort them alphabetically" }).get();


  //auto answer = engine.AskAsync({ L"I need to find out who Olivia Wilde's boyfriend is and then calculate his age raised to the 0.23 power." }).get();

  /*
auto question = L"who is Olivia Wilde's boyfriend";

auto answer = engine.AskAsync({ question }).get();

std::wcout << "Q: " << question << L"\n";
std::wcout << "A: " << answer.Value() << L"\n";
std::wcout << "Confidence: " << answer.Confidence() << L"\n";
*/



  //auto completionTask = openai.GetCompletionAsync(L"git clone ", L"text-davinci-003");
  //auto completions = completionTask.get();
  //for (auto const& c : completions) {
  //  std::wcout << c.Text() << L"\n";
  //}
  //std::wcout << L"\n\n---\n";
  auto completionTask2 = openaiEndpoint.GetCompletionAsync(
    winrt::OpenAI::builders::CompletionRequest{}
    .Prompt(L"git clone ")
    //.Model(L"text-davinci-003")
    .NCompletions(5)
    .Temperature(0.7f)
    .MaxTokens(100)
    .Stream(true)
  );
  
  auto completions2 = completionTask2.get();
  auto i = 0;
  for (auto const& c : completions2) {
    std::wcout << L"Completion #" << i << L"\n";
    std::wcout << c.Text() << L"\n";
    i++;
  }

  using namespace winrt::Windows::Foundation::Collections;
  using namespace winrt;


  auto promptTemplate = openaiEndpoint.CreateTemplate(L"Tell me a {adjective} joke about {content}");
  auto funnyJokeTask = promptTemplate.FormatAsync({ {L"adjective", L"funny"}, {L"content", L"chickens"} });
  auto funnyJoke = funnyJokeTask.get();

  std::wcout << L"\n\n" << funnyJoke << L"\n\n\n";

  auto example = openaiEndpoint.CreateFewShotTemplate({ L"word", L"antonym" });

  auto examples = std::vector {
    winrt::multi_threaded_map(std::unordered_map<hstring, hstring> { {L"word", L"happy"}, { L"antonym", L"sad" }}),
    winrt::multi_threaded_map(std::unordered_map<hstring, hstring>{ {L"word", L"tall"}, { L"antonym", L"short" }}),
    };
  example.Examples(std::move(examples));


  auto fewshot = example.ExecuteAsync(L"big").get();
  std::wcout << L"the opposite of big is " << fewshot.Lookup(L"antonym").begin() << L"\n";

  example = openaiEndpoint.CreateFewShotTemplate({ L"word", L"antonym", L"length" });
  examples = std::vector{
    winrt::multi_threaded_map(std::unordered_map<hstring, hstring> { {L"word", L"happy"}, { L"antonym", L"sad" }, { L"length", L"5" }}),
    winrt::multi_threaded_map(std::unordered_map<hstring, hstring>{ {L"word", L"tall"}, { L"antonym", L"short" }, { L"length", L"4"}}),
  };
  example.Examples(std::move(examples));
  auto word = L"huge";
  fewshot = example.ExecuteAsync(word).get();
  std::wcout << L"the opposite of " << word << L" is " << fewshot.Lookup(L"antonym").begin() << L" and the length is " << fewshot.Lookup(L"length").begin() << L"\n";

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

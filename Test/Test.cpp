// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/openai.h>
#include <winrt/builders/OpenAI.CompletionRequest.h>
#include <winrt/builders/OpenAI.OpenAIClient.h>

using namespace winrt::Windows::Foundation;

int main()
{
  winrt::init_apartment(/*winrt::apartment_type::multi_threaded*/);
  auto openai = winrt::OpenAI::builders::OpenAIClient{}
    ;


  auto emTask = openai.GetEmbeddingAsync(L"the quick brown fox");
  auto emVector = emTask.get();
  std::array<double, 1024> embedding;
  emVector.GetMany(0, winrt::array_view(embedding));
  //auto completionTask = openai.GetCompletionAsync(L"git clone ", L"text-davinci-003");
  //auto completions = completionTask.get();
  //for (auto const& c : completions) {
  //  std::wcout << c.Text() << L"\n";
  //}
  //std::wcout << L"\n\n---\n";
  auto completionTask2 = openai.GetCompletionAsync(
    winrt::OpenAI::builders::CompletionRequest{}
    .Prompt(L"git clone ")
    .Model(L"text-davinci-003")
    .NCompletions(5)
    .Temperature(0.7f)
    .MaxTokens(100)
//    .Stream(true)
  );
  
  auto completions2 = completionTask2.get();
  auto i = 0;
  for (auto const& c : completions2) {
    std::wcout << L"Completion #" << i << L"\n";
    std::wcout << c.Text() << L"\n";
    std::wcout << (uint32_t)c.FinishReason() << L"\n";
    i++;
  }

  using namespace winrt::Windows::Foundation::Collections;
  using namespace winrt;


  auto promptTemplate = openai.CreateTemplate(L"Tell me a {adjective} joke about {content}");
  auto funnyJokeTask = promptTemplate.FormatAsync({ {L"adjective", L"funny"}, {L"content", L"chickens"} });
  auto funnyJoke = funnyJokeTask.get();

  std::wcout << L"\n\n" << funnyJoke << L"\n\n\n";

  auto example = openai.CreateFewShotTemplate({ L"word", L"antonym" });

  auto examples = std::vector {
    winrt::multi_threaded_map(std::unordered_map<hstring, hstring> { {L"word", L"happy"}, { L"antonym", L"sad" }}),
    winrt::multi_threaded_map(std::unordered_map<hstring, hstring>{ {L"word", L"tall"}, { L"antonym", L"short" }}),
    };
  example.Examples(std::move(examples));


  auto fewshot = example.ExecuteAsync(L"big").get();
  std::wcout << L"the opposite of big is " << fewshot.Lookup(L"antonym").begin() << L"\n";

  example = openai.CreateFewShotTemplate({ L"word", L"antonym", L"length" });
  examples = std::vector{
    winrt::multi_threaded_map(std::unordered_map<hstring, hstring> { {L"word", L"happy"}, { L"antonym", L"sad" }, { L"length", L"5" }}),
    winrt::multi_threaded_map(std::unordered_map<hstring, hstring>{ {L"word", L"tall"}, { L"antonym", L"short" }, { L"length", L"4"}}),
  };
  example.Examples(std::move(examples));
  auto word = L"huge";
  fewshot = example.ExecuteAsync(word).get();
  std::wcout << L"the opposite of " << word << L" is " << fewshot.Lookup(L"antonym").begin() << L" and the length is " << fewshot.Lookup(L"length").begin() << L"\n";

}

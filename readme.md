# OpenAI.WinRT

WinRT library for interacting with OpenAI APIs

## Sample usage

### GetCompletionAsync
```cpp
#include <iostream>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>

#include <winrt/openai.h>
// Reference the CppWinRT.Builders NuGet package to get fluent-style property setters
#include <winrt/builders/OpenAI.CompletionRequest.h>


int main()
{
  winrt::init_apartment(/*winrt::apartment_type::multi_threaded*/);
  auto openai = winrt::OpenAI::OpenAIClient{};

  // configure the API key (otherwise it will use the value from the environment variable OPENAI_KEY)
  // openai.ApiKey(L"....");

  auto completionTask = openai.GetCompletionAsync(L"git clone ", L"text-davinci-003");
  auto completions = completionTask.get();  // you can co_await GetCompletionAsync instead if inside an async method
  for (auto const& c : completions) {
    std::wcout << c.Text() << L"\n";
  }
  std::wcout << L"\n\n---\n";
```

```cpp
  auto completionTask2 = openai.GetCompletionAsync(
    winrt::OpenAI::builders::CompletionRequest{} // this uses the CppWinRT.Builders package
    .Prompt(L"git clone ")
    .Model(L"text-davinci-003")
    .NCompletions(5)
    .Temperature(0.7f)
    .MaxTokens(100)
  );
  
  auto completions2 = completionTask2.get();
  auto i = 0;
  for (auto const& c : completions2) {
    std::wcout << L"Completion #" << i << L"\n";
    std::wcout << c.Text() << L"\n";
    i++;
  }
```

If you'd rather not use the CppWinRT.Builders NuGet package, you can set properties individually on a CompletionRequest object:
```cpp
  auto cr = winrt::OpenAI::CompletionRequest{};
  cr.Prompt(L"git clone ");
  cr.Model(L"text-davinci-003");
  cr.NCompletions(5);
  cr.Temperature(0.7f);
  cr.MaxTokens(100);
  
  auto completionTask2 = openai.GetCompletionAsync(cr);
```

### Prompt templates

```cpp
  auto promptTemplate = openai.CreateTemplate(L"Tell me a {adjective} joke about {content}");
  auto funnyJokeTask = promptTemplate.FormatAsync({ {L"adjective", L"funny"}, {L"content", L"chickens"} });
  auto funnyJoke = funnyJokeTask.get();

  std::wcout << L"\n\n" << funnyJoke << L"\n\n\n";
```

### Few-shot example templates

```cpp
  auto example = openai.CreateFewShotTemplate({ L"word", L"antonym" });

  auto examples = std::vector {
    winrt::multi_threaded_map(std::unordered_map<hstring, hstring> { {L"word", L"happy"}, { L"antonym", L"sad" }}),
    winrt::multi_threaded_map(std::unordered_map<hstring, hstring>{ {L"word", L"tall"}, { L"antonym", L"short" }}),
    };
  example.Examples(std::move(examples));


  auto fewshot = example.ExecuteAsync(L"big").get();
  std::wcout << L"the opposite of big is " << fewshot.Lookup(L"antonym").begin() << L"\n";
```
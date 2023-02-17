# OpenAI.WinRT

WinRT library for interacting with OpenAI APIs

## Sample usage

-	Create an OpenAIClient and configure it
  - To use OpenAI, specify the ApiKey, or it will use the OPENAI_KEY environment variable.
  - To use Azure OpenAI, you'll need to specify the CompletionEndpoint, the ApiKey (or use the OPENAI_KEY env var), and set UseBearerTokenAuthorization(false)
-	Once you have the client object, you can call the GetCompletionAsync API to get completions from it given a prompt. The result is an array of objects that have a Text member you can get the completion text from.

```cpp
#include <winrt/openai.h>
#include <winrt/builders/OpenAI.h>
#include <winrt/builders/helpers.h>

#ifdef USE_OPENAI // don't send any Windows or user code here

  auto openaiEndpoint = winrt::OpenAI::builders::OpenAIClient()
                      .ApiKey(L"..."); // optional or use OPENAI_KEY

#else // Azure OpenAI

  auto openaiEndpoint = winrt::OpenAI::builders::OpenAIClient()
    .CompletionUri(winrt::Windows::Foundation::Uri{ L"..." })
    .ApiKey(L"...") // optional or use OPENAI_KEY
    .UseBearerTokenAuthorization(false);

#endif

  auto completion = co_await openaiEndpoint.GetCompletionAsync(
    winrt::OpenAI::builders::CompletionRequest{}
    .Prompt(L"git clone ")
    .NCompletions(5)
    .Temperature(0.7f)
    .MaxTokens(100)
  );

  auto i = 0;
  for (auto const& c : completion) {
    std::wcout << L"Completion #" << i << L"\n";
    std::wcout << c.Text() << L"\n";
    i++;
  }
```

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

### GetEmbeddingAsync

Given a string, returns a vector of doubles representing the embedding of the string.

```cpp
  auto embedding = openai.GetEmbeddingAsync(L"hello world").get();
  for (auto const& e : embedding) {
    std::wcout << e << L", ";
  }
```

### EmbeddingDistance

Given two embeddings, returns a double representing the distance between them.

```cpp
  auto embedding1 = openai.GetEmbeddingAsync(L"hello world").get();
  auto embedding2 = openai.GetEmbeddingAsync(L"goodbye world").get();
  auto distance = openai.EmbeddingDistance(embedding1.GetView(), embedding2.GetView());
  std::wcout << L"distance between hello world and goodbye world is " << distance << L"\n";
```

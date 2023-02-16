#include "pch.h"
#include "OpenAIClient.h"
#include "OpenAIClient.g.cpp"
#include "CompletionRequest.h"
#include "PromptTemplate.h"
#include "EmbeddingUtils.g.cpp"
#include <winrt/Windows.Web.Http.h>
#include <winrt/Windows.Web.Http.Headers.h>
#include <winrt/Windows.Data.Json.h>
#include <format>
namespace winrt {
  using namespace Windows::Data::Json;
  using namespace Windows::Foundation;
  using namespace Windows::Foundation::Collections;
  using namespace Windows::Storage::Streams;
  using namespace Windows::Web::Http;
}

namespace winrt::OpenAI::implementation
{
  OpenAIClient::OpenAIClient()
  {
    wchar_t buffer[100]{};
    if (GetEnvironmentVariable(L"OPENAI_KEY", buffer, static_cast<DWORD>(std::size(buffer))) != 0) {
      ApiKey(buffer);
    }
  }

  void OpenAIClient::ApiKey(winrt::hstring v) noexcept
  {
    m_apiKey = v;
    SetAuth();
  }

  void OpenAIClient::SetAuth() {
    m_client.DefaultRequestHeaders().Clear();
    if (m_useBearerTokenAuthorization) {
      m_client.DefaultRequestHeaders().Authorization(winrt::Headers::HttpCredentialsHeaderValue(L"Bearer", m_apiKey));
    } else {
      m_client.DefaultRequestHeaders().Append(L"api-key", m_apiKey);
    }
  }

  winrt::IAsyncOperation<winrt::IVector<winrt::OpenAI::Choice>> OpenAIClient::GetCompletionAsync(winrt::hstring prompt, winrt::hstring model)
  {
    auto request = winrt::make<CompletionRequest>();
    request.Prompt(prompt);
    request.Model(model);

    return GetCompletionAsync(request);
  }
  winrt::hstring EscapeStringForJson(winrt::hstring v) {
    return winrt::JsonValue::CreateStringValue(v).Stringify();
  }
  winrt::IAsyncOperation<winrt::IVector<winrt::OpenAI::Choice>> OpenAIClient::GetCompletionAsync(winrt::OpenAI::CompletionRequest request)
  {
    std::vector<winrt::OpenAI::Choice> retChoices;
    HttpStatusCode statusCode{};
#ifdef _DEBUG
    try
#endif
    {
      auto promptString = EscapeStringForJson(request.Prompt());
      auto modelString = EscapeStringForJson(request.Model());

      constexpr std::wstring_view requestTemplate{ LR"({{
  "model": {},
  "prompt": {},
  "temperature": {},
  "max_tokens": {},
  "n": {},
  "top_p": {},
  "stream": {}
}})" };
      const std::wstring_view model{ modelString };
      const std::wstring_view prompt{ promptString };
      auto requestJson = std::vformat(requestTemplate, std::make_wformat_args(
        model, prompt, request.Temperature(), request.MaxTokens(), request.NCompletions(), request.TopP(),
        request.Stream() ? L"true" : L"false"
        ));
      auto content = winrt::HttpStringContent(requestJson, winrt::UnicodeEncoding::Utf8, L"application/json");
      auto response = co_await m_client.PostAsync(CompletionUri(), content);
      auto responseJsonStr = co_await response.Content().ReadAsStringAsync();
      statusCode = response.StatusCode();

      response.EnsureSuccessStatusCode();
      if (!request.Stream()) {
        auto responseJson = JsonObject::Parse(responseJsonStr);
        auto choices = responseJson.GetNamedArray(L"choices");
        for (const auto& c : choices) {
          const auto& choice = c.GetObject();
          auto retChoice = winrt::make<Choice>();
          auto retChoiceImpl = winrt::get_self<Choice>(retChoice);
          retChoiceImpl->m_text = choice.GetNamedString(L"text");
          auto finish_reason = choice.GetNamedString(L"finish_reason");
          if (finish_reason == L"stop") {
            retChoiceImpl->m_finishReason = FinishReason::Stop;
          } else if (finish_reason == L"length") {
            retChoiceImpl->m_finishReason = FinishReason::Length;
          } else {
            throw winrt::hresult_invalid_argument{};
          }
          retChoices.push_back(retChoice);
        }
      } else {
        auto ptr = responseJsonStr.begin();
        std::vector<std::wstring> built;
        while (ptr < responseJsonStr.end() && ptr != nullptr) {
          constexpr wchar_t dataStr[] = L"data: ";
          constexpr wchar_t doneStr[] = L"[DONE]";
          if (wcsncmp(ptr, dataStr, std::size(dataStr) - 1) == 0) {
            ptr += std::size(dataStr) - 1;
            auto jsonStr = ptr;
            ptr = wcschr(jsonStr, L'\n') + 1; // double \n
            const_cast<wchar_t*>(ptr)[0] = L'\0';
            ptr++;
            if (wcsncmp(jsonStr, doneStr, std::size(doneStr) - 1) == 0) break;
            auto json = JsonObject::Parse(jsonStr);
            auto choices = json.GetNamedArray(L"choices");
            auto choice = choices.GetObjectAt(0);
            auto text = choice.GetNamedString(L"text");
            auto index = choice.GetNamedNumber(L"index");
            if (built.size() <= index) {
              built.reserve(index + 1);
              for (auto i = built.size(); i <= index; i++) {
                built.push_back({});
              }
            }
            built[index] += text;
          } else {
            throw winrt::hresult_out_of_bounds();
          }
        }
        for (const auto& c : built) {
          auto retChoice = winrt::make<Choice>();
          auto retChoiceImpl = winrt::get_self<Choice>(retChoice);
          retChoiceImpl->m_text = c;
          retChoices.push_back(retChoice);
        }
      }
    }
#ifdef _DEBUG      
    catch (std::exception& e) {
      auto x = e.what();
      throw;
    }
    catch (winrt::hresult_error& e) {
      auto x = e.message();
      switch (statusCode) {
      case winrt::Windows::Web::Http::HttpStatusCode::None:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::Continue:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::SwitchingProtocols:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::Processing:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::Ok:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::Created:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::Accepted:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::NonAuthoritativeInformation:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::NoContent:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::ResetContent:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::PartialContent:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::MultiStatus:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::AlreadyReported:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::IMUsed:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::MultipleChoices:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::MovedPermanently:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::Found:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::SeeOther:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::NotModified:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::UseProxy:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::TemporaryRedirect:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::PermanentRedirect:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::BadRequest:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::Unauthorized:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::PaymentRequired:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::Forbidden:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::NotFound:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::MethodNotAllowed:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::NotAcceptable:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::ProxyAuthenticationRequired:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::RequestTimeout:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::Conflict:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::Gone:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::LengthRequired:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::PreconditionFailed:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::RequestEntityTooLarge:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::RequestUriTooLong:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::UnsupportedMediaType:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::RequestedRangeNotSatisfiable:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::ExpectationFailed:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::UnprocessableEntity:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::Locked:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::FailedDependency:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::UpgradeRequired:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::PreconditionRequired:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::TooManyRequests:
        throw winrt::hresult_error(HRESULT_FROM_NT(RPC_S_SERVER_TOO_BUSY));
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::RequestHeaderFieldsTooLarge:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::InternalServerError:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::NotImplemented:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::BadGateway:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::ServiceUnavailable:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::GatewayTimeout:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::HttpVersionNotSupported:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::VariantAlsoNegotiates:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::InsufficientStorage:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::LoopDetected:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::NotExtended:
        break;
      case winrt::Windows::Web::Http::HttpStatusCode::NetworkAuthenticationRequired:
        break;
      default:
        break;
      }
      throw;
    }
#endif
    auto ret = winrt::single_threaded_vector<winrt::OpenAI::Choice>(std::move(retChoices));
    co_return ret;

  }

  winrt::OpenAI::PromptTemplate OpenAIClient::CreateTemplate(winrt::hstring promptTemplateString) {
    auto promptTemplate = winrt::make<PromptTemplate>();
    auto promptTemplateImpl = winrt::get_self<PromptTemplate>(promptTemplate);
    promptTemplateImpl->m_client = *this;
    promptTemplateImpl->Template(promptTemplateString);
    return promptTemplate;
  }

  winrt::OpenAI::FewShotTemplate OpenAIClient::CreateFewShotTemplate(winrt::Windows::Foundation::Collections::IVectorView<winrt::hstring> parameters)
  {
    auto promptTemplate = winrt::make<FewShotTemplate>();
    auto promptTemplateImpl = winrt::get_self<FewShotTemplate>(promptTemplate);
    promptTemplateImpl->m_client = *this;
    for (const auto& p : parameters) {
      promptTemplateImpl->m_parameters.push_back(p.c_str());
    }
    return promptTemplate;

  }

  Windows::Foundation::IAsyncOperation<winrt::Windows::Foundation::Collections::IVector<double>> OpenAIClient::GetEmbeddingAsync(winrt::hstring prompt) {
    auto input = JsonValue::CreateStringValue(prompt);
    auto request = JsonObject();
    request.Insert(L"input", input);
    auto reqStr = request.Stringify();
    auto content = winrt::HttpStringContent(reqStr, winrt::UnicodeEncoding::Utf8, L"application/json");
    auto response = co_await m_client.PostAsync(EmbeddingUri(), content);
    auto responseJsonStr = co_await response.Content().ReadAsStringAsync();
    auto statusCode = response.StatusCode();

    response.EnsureSuccessStatusCode();
    auto responseJson = JsonObject::Parse(responseJsonStr);
    auto data = responseJson.GetNamedArray(L"data");
    auto first = data.GetObjectAt(0);
    auto embedding = first.GetNamedArray(L"embedding");
    std::vector<double> values;
    values.reserve(1024);
    try {
      for (const auto& v : embedding) values.push_back(v.GetNumber());
    }
    catch (const winrt::hresult_error& e) { auto x = e.message(); }
    co_return winrt::single_threaded_vector(std::move(values));
  }


  struct L1Similarity {
    static double ElementDistance(double x, double y, double accumulatedDistance) { return accumulatedDistance + std::abs(x - y); }
    static double VectorDistance(double accumulatedDistance, double norm1, double norm2) { return accumulatedDistance / (norm1 * norm2); }
    using norm_t = L1Similarity;
  };
  struct L2Similarity {
    static double ElementDistance(double x, double y, double accumulatedDistance) { return accumulatedDistance + std::pow(x - y, 2); }
    static double VectorDistance(double accumulatedDistance, double norm1, double norm2) { return std::sqrt(accumulatedDistance / (norm1 * norm2)); }
    using norm_t = L2Similarity;
  };
  struct LinfSimilarity {
    static double ElementDistance(double x, double y, double accumulatedDistance) { return (std::max)(accumulatedDistance, std::abs(x-y)); }
    static double VectorDistance(double accumulatedDistance, double norm1, double norm2) { return accumulatedDistance / (norm1 * norm2); }
    using norm_t = LinfSimilarity;
  };
  struct CosineSimilarity {
    static double ElementDistance(double x, double y, double accumulatedDistance) { return accumulatedDistance + x * y; }
    static double VectorDistance(double accumulatedDistance, double norm1, double norm2) { return accumulatedDistance / std::sqrt(norm1 * norm2); }
    using norm_t = L2Similarity;
  };
  template<typename T>
  double EmbeddingDistanceImpl(winrt::Windows::Foundation::Collections::IVectorView<double> const& v1, winrt::Windows::Foundation::Collections::IVectorView<double> const& v2)
  {
    double distance = 0;
    double norm1 = 0, norm2 = 0;
    for (uint32_t i = 0; i < v1.Size(); i++) {
      distance = T::ElementDistance(v1.GetAt(i), v2.GetAt(i), distance);
      norm1 = T::norm_t::ElementDistance(v1.GetAt(i), 0, norm1);
      norm2 = T::norm_t::ElementDistance(v2.GetAt(i), 0, norm2);
    }
    return T::VectorDistance(distance, norm1, norm2);
  }

  double EmbeddingUtils::EmbeddingDistance(winrt::Windows::Foundation::Collections::IVectorView<double> const& v1, winrt::Windows::Foundation::Collections::IVectorView<double> const& v2, winrt::OpenAI::Similarity const& similarity)
  {
    if (v1.Size() != v2.Size()) throw winrt::hresult_invalid_argument{};
    switch (similarity) {
    case Similarity::Cosine:
      return EmbeddingDistanceImpl<CosineSimilarity>(v1, v2);
    case Similarity::L1:
      return EmbeddingDistanceImpl<L1Similarity>(v1, v2);
    case Similarity::L2:
      return EmbeddingDistanceImpl<L2Similarity>(v1, v2);
    case Similarity::Linf:
      return EmbeddingDistanceImpl<LinfSimilarity>(v1, v2);
    }
    throw winrt::hresult_invalid_argument{};
  }
  double EmbeddingUtils::EmbeddingDistance(winrt::Windows::Foundation::Collections::IVectorView<double> const& v1, winrt::Windows::Foundation::Collections::IVectorView<double> const& v2)
  {
    return EmbeddingDistance(v1, v2, Similarity::Cosine);
  }

}

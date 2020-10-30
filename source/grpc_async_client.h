// Copyright 2020 SkyAPM

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <grpcpp/grpcpp.h>

#include <cstddef>
#include <list>
#include <memory>

#include "common/Common.pb.h"
#include "cpp2sky/internal/async_client.h"
#include "external/skywalking_data_collect_protocol/language-agent/Tracing.pb.h"
#include "language-agent/Tracing.grpc.pb.h"
#include "language-agent/Tracing.pb.h"
#include "source/utils/status.h"

namespace cpp2sky {

class GrpcAsyncSegmentReporterStream;

class GrpcAsyncSegmentReporterClient : public AsyncClient {
 public:
  GrpcAsyncSegmentReporterClient(grpc::CompletionQueue& cq,
                                 std::shared_ptr<grpc::Channel> channel,
                                 AsyncStreamFactoryPtr factory);

  void onSendMessage(const Message& message) override;
  size_t numOfStreams() const override { return streams_.size(); }

 private:
  grpc::CompletionQueue& cq_;
  TraceSegmentReportService::Stub stub_;
  std::list<AsyncStreamPtr> streams_;
  grpc::ClientContext ctx_;
  AsyncStreamFactoryPtr factory_;

  friend class GrpcAsyncSegmentReporterStream;
};

class GrpcAsyncSegmentReporterStream : public AsyncStream {
 public:
  GrpcAsyncSegmentReporterStream(GrpcAsyncSegmentReporterClient* parent)
      : parent_(parent) {}

  uint16_t status() const override {
    return grpcStatusToGenericHttpStatus(status_.error_code());
  }

  void startStream() override;
  void setData(const Message& message) override;
  const Message& reply() const override;

 private:
  bool data_set_{false};
  SegmentObject data_;
  GrpcAsyncSegmentReporterClient* parent_;
  grpc::Status status_;
  Commands commands_;
  std::unique_ptr<grpc::ClientAsyncWriter<SegmentObject>> request_writer_;
};

class GrpcAsyncSegmentReporterStreamFactory : public AsyncStreamFactory {
 public:
  explicit GrpcAsyncSegmentReporterStreamFactory(
      GrpcAsyncSegmentReporterClient* client);

  AsyncStreamPtr create() override;

 private:
  GrpcAsyncSegmentReporterClient* client_;
};

}  // namespace cpp2sky

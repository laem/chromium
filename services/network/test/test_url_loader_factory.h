// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_NETWORK_TEST_TEST_URL_LOADER_FACTORY_H_
#define SERVICES_NETWORK_TEST_TEST_URL_LOADER_FACTORY_H_

#include <vector>

#include "base/macros.h"
#include "services/network/public/mojom/url_loader_factory.mojom.h"

namespace network {

// A helper class to ease testing code that uses URLLoader interface. A test
// would pass this factory instead of the production factory to code, and
// would prime it with response data for arbitrary URLs.
class TestURLLoaderFactory : public mojom::URLLoaderFactory {
 public:
  TestURLLoaderFactory();
  ~TestURLLoaderFactory() override;

  using Redirects =
      std::vector<std::pair<net::RedirectInfo, ResourceResponseHead>>;

  // Adds a response to be served. Once served it will be removed, so if you
  // expect n requests to the same URL call this method n times.
  // This can be called before or after a request is made. If it's called after,
  // then pending requests will be "woken up".
  void AddResponse(const GURL& url,
                   const ResourceResponseHead& head,
                   const std::string& content,
                   const URLLoaderCompletionStatus& status,
                   const Redirects& redirects = Redirects());

  // Simpler version of above for the common success case.
  void AddResponse(const std::string& url, const std::string& content);

  // mojom::URLLoaderFactory implementation.
  void CreateLoaderAndStart(mojom::URLLoaderRequest request,
                            int32_t routing_id,
                            int32_t request_id,
                            uint32_t options,
                            const ResourceRequest& url_request,
                            mojom::URLLoaderClientPtr client,
                            const net::MutableNetworkTrafficAnnotationTag&
                                traffic_annotation) override;
  void Clone(mojom::URLLoaderFactoryRequest request) override;

 private:
  bool CreateLoaderAndStartInternal(const GURL& url,
                                    mojom::URLLoaderClient* client);

  struct Response {
    Response();
    ~Response();
    Response(const Response&);
    GURL url;
    Redirects redirects;
    ResourceResponseHead head;
    std::string content;
    URLLoaderCompletionStatus status;
  };
  std::vector<Response> responses_;

  struct Pending {
    Pending();
    ~Pending();
    Pending(Pending&& other);
    Pending& operator=(Pending&& other);
    GURL url;
    mojom::URLLoaderClientPtr client;
  };
  std::vector<Pending> pending_;

  DISALLOW_COPY_AND_ASSIGN(TestURLLoaderFactory);
};

}  // namespace network

#endif  // SERVICES_NETWORK_TEST_TEST_URL_LOADER_FACTORY_H_

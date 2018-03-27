/*
// Copyright (c) 2018 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#include "framebuffermanager.h"

#include "platformcommondefines.h"

namespace hwcomposer {

FrameBufferManager *FrameBufferManager::pInstance = NULL;

FrameBufferManager *FrameBufferManager::GetInstance() {
  if (pInstance == NULL)
    pInstance = new FrameBufferManager();
  return pInstance;
}

uint32_t FrameBufferManager::FindFB(const FBKey &key) {
  lock_.lock();
  auto it = fb_map_.find(key);
  if (it != fb_map_.end()) {
    it->second.fb_ref++;
    lock_.unlock();
    return it->second.fb_id;
  } else {
    uint32_t fb_id = 0;
    int ret = CreateFrameBuffer(key, &fb_id);
    if (ret) {
      lock_.unlock();
      return fb_id;
    }

    FBValue value;
    value.fb_id = fb_id;
    value.fb_ref = 1;
    fb_map_.emplace(std::make_pair(key, value));
    lock_.unlock();
    return fb_id;
  }
}

int FrameBufferManager::RemoveFB(const uint32_t fb) {
  lock_.lock();
  auto it = fb_map_.begin();
  int ret = 0;

  while (it != fb_map_.end()) {
    if (it->second.fb_id == fb) {
      it->second.fb_ref -= 1;
      if (it->second.fb_ref == 0) {
        ret = ReleaseFrameBuffer(it->first, fb);
        fb_map_.erase(it);
      }
      break;
    }
    it++;
  }

  lock_.unlock();
  return ret;
}

void FrameBufferManager::PurgeAllFBs() {
  lock_.lock();
  auto it = fb_map_.begin();

  while (it != fb_map_.end()) {
    ReleaseFrameBuffer(it->first, it->second.fb_id);
  }

  lock_.unlock();
}

}  // namespace hwcomposer

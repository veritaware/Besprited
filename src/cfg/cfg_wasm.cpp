// Config Library
// Besprited | Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef __EMSCRIPTEN__

#include "cfg/cfg_wasm.h"

#include "she/system.h"

#include <emscripten/emscripten.h>

#include <cstdlib>

// The JS below lives in its own file (listed in .clang-format-ignore) because
// clang-format still parses guarded code and mis-indents whatever follows a
// `name(){ stmt; }` JS method.

static std::string s_cfgdata;

static void cfgwrite()
{
  she::instance()->gfx([&] {
    EM_ASM((
      self.storage.data = JSON.parse(UTF8ToString($0));
      self.storage.dirty = true;
      self.storage.commit();
    ), s_cfgdata.c_str());
  });
}

static void thread_init()
{
  EM_ASM((
    self.storage = {
    data: JSON.parse(UTF8ToString($0)),
    getItem(key){ return storage.data[key]; },
    setItem(key, value){ storage.data[key] = value; }
    };
  ), s_cfgdata.c_str());
}

bool cfginit()
{
  auto data = (char*) EM_ASM_PTR((
      if (self.storage)
  return self.storage.data ? stringToNewUTF8(JSON.stringify(self.storage.data)) : 0;
      self.storage = {
        data:null,
  db:null,
  dirty:false,
  init(){
    if (storage.wasInit)
        return;
    storage.wasInit = true;

    Object.assign(indexedDB.open("UserSettings", 1), {
      onupgradeneeded(){
        storage.db = this.result;
        console.log(storage.db);
        storage.db.createObjectStore('userSettings', {keyPath: 'key'});
      },
      onerror(){},
      onsuccess(){
        storage.db = this.result;
        let transaction = storage.db.transaction('userSettings', 'readonly');
        let userSettings = transaction.objectStore('userSettings');
        Object.assign(userSettings.get('str'), {
    onsuccess(){
      storage.data = (this.result ?? {data:{}}).data;
    }
        });
      }
    });
  },
  commit(){
    if (!storage.dirty) return;
    let transaction = storage.db.transaction('userSettings', 'readwrite');
    let userSettings = transaction.objectStore('userSettings');
    Object.assign(userSettings.put({key:'str', data:storage.data}), {
      onsuccess(){storage.dirty = false;},
      onerror(){}
    });
  },
  getItem(key){
    console.log(key);
    return storage.data[key];
  },
  setItem(key, value){
    console.log(key, value);
    storage.dirty = storage.data[key] != value;
    storage.data[key] = value;
  }
      };
      storage.init();
      return 0;
    ));

  if (!data)
      return false;

  s_cfgdata = data;
  free(data);
  return true;
}

    std::string cfg_wasm_load(const std::string& filename)
{
  thread_init();
  auto raw = (char*) EM_ASM_PTR({
    const value = self.storage.getItem(UTF8ToString($0));
    if (value === undefined)
      return 0;
    return stringToNewUTF8(value);
  }, filename.c_str());
  std::string data;
  if (raw)
  {
    data = raw;
    free(raw);
  }
  return data;
}

void cfg_wasm_save(const std::string& filename, const std::string& data)
{
  thread_init();
  auto cfgdata = (char*) EM_ASM_PTR({
    self.storage.setItem(UTF8ToString($0), UTF8ToString($1));
    return stringToNewUTF8(JSON.stringify(self.storage.data));
  }, filename.c_str(), data.c_str());
  if (cfgdata)
  {
    s_cfgdata = cfgdata;
    free(cfgdata);
    cfgwrite();
  }
}

#endif // __EMSCRIPTEN__

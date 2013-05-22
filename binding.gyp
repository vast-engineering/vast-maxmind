{
  "targets": [
    {
      "target_name": "index",
      "sources": [ "index.cc", "vastmaxmind.cc" ],
      'link_settings': {
          'libraries': [
              '-lGeoIP'
          ]
      }
    }
  ]
}
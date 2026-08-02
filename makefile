
define package
{
  login: vthierry
  logo: "docs/idnai-logo-green.png"
  keywords: [ web-service weak-json ]
  dependencies: [ idnai-make ]
  os: [ Linux armv7l esp32 mingw64 ]
}
endef

include node_modules/idnai-make/src/makefile-rules.mk

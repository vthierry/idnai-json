
define package
{
  login: vthierry
  logo: "docs/logos/idnai-logo-green.png"
  keywords: [ web-service weak-json ]
  dependencies: [ idnai-make ]
}
endef

include node_modules/idnai-make/src/makefile-rules.mk

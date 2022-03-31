#pragma once

#include "ETcpPluginSettings.generated.h"

UCLASS(config = Engine)
class ETCPPLUGIN_API UETcpPluginSettings: public UObject {

	GENERATED_BODY()

public:

	UPROPERTY(Config, EditAnywhere, Category = "ETcpPluginSettings")
	bool bPostErrorsToMessageLog;
};
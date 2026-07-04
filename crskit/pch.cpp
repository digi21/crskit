#include "pch.h"
#include "CoordinateSystems/CoordinateSystemAuthorityFactory.h"
#include "CoordinateTransformations/CoordinateTransformationAuthorityFactory.h"
#include "CrsContext.h"
#include "CoordinateTransformations/MathTransformFactory.h"
#include "CoordinateTransformations/CoordinateTransformationFactory.h"
#include "CoordinateTransformations/MathTransformRegistry.h"
#include "Utilities.h"

namespace {
	std::mutex gEnvironmentMutex;
	std::shared_ptr<CrsKit::Environment> gEnvironment;

	// Returns the current environment, lazily creating a minimal default one (with a default CrsContext)
	// if none has been set yet, so GetDefaultContext() and friends work before Initialize().
	auto currentEnvironment() -> std::shared_ptr<CrsKit::Environment>
	{
		std::lock_guard const lock{ gEnvironmentMutex };
		if (!gEnvironment)
			gEnvironment = std::make_shared<CrsKit::Environment>();
		if (!gEnvironment->DefaultContext)
			gEnvironment->DefaultContext = std::make_shared<CrsKit::CrsContext>();
		return gEnvironment;
	}
}

namespace CrsKit
{
	std::shared_ptr<Environment> GetCurrentEnvironment()
	{
		return currentEnvironment();
	}

	void SetCurrentEnvironment(std::shared_ptr<Environment> const& environment)
	{
		std::lock_guard const lock{ gEnvironmentMutex };
		gEnvironment = environment;
	}

	std::shared_ptr<CrsContext> GetDefaultContext()
	{
		return currentEnvironment()->DefaultContext;
	}

	bool Initialize(std::shared_ptr<Epsg::IAuthorityProvider> const& authorityProvider)
	{
		try {
			RegisterBuiltInMathTransforms();

			auto environment = std::make_shared<Environment>();
			// Preserve any default context already set (e.g. dataDirectory) before Initialize().
			environment->DefaultContext = GetDefaultContext();
			environment->Provider = authorityProvider;
			environment->CoordinateSystemAuthorityFactory = std::make_shared<Epsg::CoordinateSystemAuthorityFactory>(authorityProvider);
			environment->CoordinateSystemFactory = std::make_shared<CoordinateSystems::CoordinateSystemFactory>();
			environment->MathTransformFactory = std::make_shared<CoordinateTransformations::MathTransformFactory>();
			environment->CoordinateTransformationAuthorityFactory = std::make_shared<Epsg::CoordinateTransformationAuthorityFactory>(authorityProvider);
			environment->CoordinateTransformationFactory = std::make_shared<CoordinateTransformations::CoordinateTransformationFactory>();
			environment->Utilities = std::make_shared<Utilities>();

			std::lock_guard const lock{ gEnvironmentMutex };
			gEnvironment = std::move(environment);
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	std::shared_ptr<CoordinateSystems::ICoordinateSystemFactory> GetCoordinateSystemFactory()
	{
		return currentEnvironment()->CoordinateSystemFactory;
	}

	std::shared_ptr<CoordinateTransformations::IMathTransformFactory> GetMathTransformFactory()
	{
		return currentEnvironment()->MathTransformFactory;
	}

	std::shared_ptr<CoordinateSystems::ICoordinateSystemAuthorityFactory> GetCoordinateSystemAuthorityFactory()
	{
		return currentEnvironment()->CoordinateSystemAuthorityFactory;
	}

	std::shared_ptr<CoordinateTransformations::ICoordinateTransformationAuthorityFactory> GetCoordinateTransformationAuthorityFactory()
	{
		return currentEnvironment()->CoordinateTransformationAuthorityFactory;
	}

	std::shared_ptr<CoordinateTransformations::ICoordinateTransformationFactory> GetCoordinateTransformationFactory()
	{
		return currentEnvironment()->CoordinateTransformationFactory;
	}

	std::shared_ptr<IUtilities> GetUtilities()
	{
		return currentEnvironment()->Utilities;
	}
}

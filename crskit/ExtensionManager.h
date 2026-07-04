#pragma once

#include <memory>
#include <string>
#include <typeindex>
#include <vector>

#include "OpenGisAttributes.h"
#include "CoordinateSystems/IProjection.h"
#include "CoordinateTransformations/IMathTransform.h"

namespace CrsKit
{
	/// <summary>
	///		Finds and instantiates types decorated with the <c>AuthorityOperation</c> attribute.
	/// </summary>
	///	<remarks>
	///		All types implementing mathematical operations defined by an authority, such as EPSG, are decorated with the AuthorityOperation type. 
	///	</remarks>
	class ExtensionManager final
	{

	public:
		/// <summary>
		///		Finds the type that implements a given code for a given authority.
		///	</summary>
		///	<param name="authority">
		///		Name of the authority for which the type is being located.
		///	</param>
		///	<param name="code">
		///		Code of the mathematical operation to locate.
		///	</param>
		///	<remarks>
		///		Used by the method: CoordinateSystemAuthorityFactory::CreateProjection which passes "EPSG" as the first parameter.
		///	</remarks>
		///	<exception cref="Exception">Thrown if no type implementing the requested mathematical operation is found.</exception>
		///	<exception cref="Exception">Thrown if no type implementing the requested mathematical operation is found.</exception>
		static auto FindMathematicalOperation(std::string const& authority, int code) -> std::type_index;

		static auto GetMathOperationParameters(std::type_index const& operation) -> std::vector<std::shared_ptr<WktParameterAttribute>>;
		static auto GetClassificationName(std::type_index const& operation) -> std::string;

		///	<summary>
		///		Instantiates a mathematical operation.
		///	</summary>
		///	<param name="parameters">Type indicating the OpenGis name of the mathematical operation to instantiate as well as its parameters.</param>
		///	<param name="inverse">Boolean indicating whether to instantiate the direct or the inverse operation.</param>
		///	<exception cref="Excepton">
		///		Thrown if a type matching the OpenGis name passed as parameters is found but does not have a constructor compatible with the implementation.
		///	<exception>
		///	<exception cref="Excepton">
		///		Thrown if no type matching the OpenGis name passed as parameters is found.
		///	<exception>
		static auto CreateMathTransform(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse) -> std::shared_ptr<CoordinateTransformations::IMathTransform>;
	};
}

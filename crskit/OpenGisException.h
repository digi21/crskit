#pragma once

#include <stdexcept>
#include <string>

#include "CrsKitExport.h"

namespace CrsKit
{
	///	<summary>
	///		Library domain exception. Replaces the old 'throw 1'.
	///	</summary>
	class _visibleCrsKit OpenGisException : public std::runtime_error
	{
	public:
		explicit OpenGisException(std::string const& message = "Digi21.OpenGis error")
			: std::runtime_error{ message }
		{
		}
	};

	// Typed library exceptions, all derived from OpenGisException so existing catch(std::exception) /
	// catch(std::runtime_error) / catch(OpenGisException) keep working; they only add the option of
	// catching a specific failure by type. Message-only for now (the value is the type).

	// An EPSG/authority code (or the object it names) could not be located.
	class _visibleCrsKit AuthorityCodeNotFoundException : public OpenGisException
	{
	public:
		using OpenGisException::OpenGisException;
	};

	// A WKT string is malformed: a required element is missing or out of place.
	class _visibleCrsKit WktParseException : public OpenGisException
	{
	public:
		using OpenGisException::OpenGisException;
	};

	// No coordinate operation (or more than one, ambiguous) was found between two coordinate systems.
	class _visibleCrsKit TransformationNotFoundException : public OpenGisException
	{
	public:
		using OpenGisException::OpenGisException;
	};

	// A point or coordinate buffer does not have the dimensions the transform expects.
	class _visibleCrsKit DimensionMismatchException : public OpenGisException
	{
	public:
		using OpenGisException::OpenGisException;
	};

	// A coordinate lies outside the area a grid covers or a projection's valid domain.
	class _visibleCrsKit CoordinateOutsideDomainException : public OpenGisException
	{
	public:
		using OpenGisException::OpenGisException;
	};

	// A file format, algorithm or operation method is not supported.
	class _visibleCrsKit UnsupportedFormatException : public OpenGisException
	{
	public:
		using OpenGisException::OpenGisException;
	};
}

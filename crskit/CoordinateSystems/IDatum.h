#pragma once

namespace CrsKit::CoordinateSystems
{
	struct IDatum
	{
	protected:
		int _datumType;

		IDatum(int datumType) : _datumType{datumType}
		{
		}

	public:
		[[nodiscard]] virtual auto GetDatumType() const -> int { return _datumType; }
	};
}

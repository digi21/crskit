#pragma once

namespace CrsKit::CoordinateSystems
{
	struct IVerticalDatum
	{
		virtual auto MetodoVirtualParaQueEsteTipoSeConsiderePolimorfico() -> void
		{
		};
	};
}

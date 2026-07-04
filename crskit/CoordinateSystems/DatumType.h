#pragma once

namespace CrsKit::CoordinateSystems
{
	struct DatumType final
	{
		static constexpr int HD_Min = 1000;
		static constexpr int HD_Other = 1000;
		static constexpr int HD_Classic = 1001;
		static constexpr int HD_Geocentric = 1002;
		static constexpr int HD_Max = 1999;
		static constexpr int VD_Min = 2000;
		static constexpr int VD_Other = 2000;
		static constexpr int VD_Orthometric = 2001;
		static constexpr int VD_Ellipsoidal = 2002;
		static constexpr int VD_AltitudeBarometric = 2003;
		static constexpr int VD_Normal = 2004;
		static constexpr int VD_GeoidModelDerived = 2005;
		static constexpr int VD_Depth = 2006;
		static constexpr int VD_Max = 2999;
		static constexpr int LD_Min = 10000;
		static constexpr int LD_Max = 32767;
	};
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace LearninkKit.Net.Lib
{
    public enum KIT_SYMBOL
    {
        Undefined = 0x0,
        Zero = 0xD7,
        One = 0x6,
        Two = 0xB3,
        Three = 0xA7,
        Four = 0x66,
        Five = 0xE5,
        Six = 0xF5,
        Seven = 0x07,
        Eight = 0xF7,
        Nine = 0xE7,
        /*Ten,
        A,
        C,
        E,
        F,
        H,
        L,
        P,
        S,
        U*/
    }

    [Flags]
    public enum KIT_BAR : byte
    {
        Empty = 0x0,
        Bar1 = 0x20,
        Bar2 = 0x40,
        Bar3 = 0x80,
        Bar4 = 0x01,
        Bar5 = 0x02,
        Bar6 = 0x04,
        Bar7 = 0x08,
        Bar8 = 0x10
    }

    public class Wrapper
    {
        public bool DisplaySymbol(KIT_SYMBOL symbol)
        {
            return Native_DisplaySymbol(symbol);
        }

        public KIT_SYMBOL GetDisplayedSymbol()
        {
            return Native_GetDisplayedSymbol();
        }

        public bool LightBars(KIT_BAR bars)
        {
            return Native_LightBars(bars);
        }

        public KIT_BAR GetLightBars()
        {
            return Native_GetLightBars();
        }

        [DllImport(@"LearningKit.Lib.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint = "DisplaySymbol")]
        static extern bool Native_DisplaySymbol(KIT_SYMBOL symbol);

        [DllImport(@"LearningKit.Lib.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint = "GetDisplayedSymbol")]
        static extern KIT_SYMBOL Native_GetDisplayedSymbol();

        [DllImport(@"LearningKit.Lib.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint = "LightBars")]
        static extern bool Native_LightBars(KIT_BAR bar);

        [DllImport(@"LearningKit.Lib.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint = "GetLightBars")]
        static extern KIT_BAR Native_GetLightBars();
    }
}

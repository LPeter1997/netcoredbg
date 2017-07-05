#ifndef IfFailRet
#define IfFailRet(EXPR) do { Status = (EXPR); if(FAILED(Status)) { return (Status); } } while (0)
#endif

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof(x[0]))
#endif

#ifdef PAL_STDCPP_COMPAT
#define _iswprint   PAL_iswprint
#define _wcslen     PAL_wcslen
#define _wcsncmp    PAL_wcsncmp
#define _wcsrchr    PAL_wcsrchr
#define _wcscmp     PAL_wcscmp
#define _wcschr     PAL_wcschr
#define _wcscspn    PAL_wcscspn
#define _wcscat     PAL_wcscat
#define _wcsstr     PAL_wcsstr
#else // PAL_STDCPP_COMPAT
#define _iswprint   iswprint
#define _wcslen     wcslen
#define _wcsncmp    wcsncmp
#define _wcsrchr    wcsrchr
#define _wcscmp     wcscmp
#define _wcschr     wcschr
#define _wcscspn    wcscspn
#define _wcscat     wcscat
#define _wcsstr     wcsstr
#endif // !PAL_STDCPP_COMPAT

typedef uintptr_t TADDR;
typedef ULONG64 CLRDATA_ADDRESS;

// Convert between CLRDATA_ADDRESS and TADDR.
#define TO_TADDR(cdaddr) ((TADDR)(cdaddr))
#define TO_CDADDR(taddr) ((CLRDATA_ADDRESS)(LONG_PTR)(taddr))

const int mdNameLen = 2048;

static const char *SymbolReaderDllName = "SOS.NETCore";
static const char *SymbolReaderClassName = "SOS.SymbolReader";

typedef  int (*ReadMemoryDelegate)(ULONG64, char *, int);
typedef  PVOID (*LoadSymbolsForModuleDelegate)(const char*, BOOL, ULONG64, int, ULONG64, int, ReadMemoryDelegate);
typedef  void (*DisposeDelegate)(PVOID);
typedef  BOOL (*ResolveSequencePointDelegate)(PVOID, const char*, unsigned int, unsigned int*, unsigned int*);
typedef  BOOL (*GetLocalVariableName)(PVOID, int, int, BSTR*);
typedef  BOOL (*GetLineByILOffsetDelegate)(PVOID, mdMethodDef, ULONG64, ULONG *, BSTR*);
typedef  BOOL (*GetStepRangesFromIPDelegate)(PVOID, int, mdMethodDef, unsigned int*, unsigned int*);

BOOL SafeReadMemory (TADDR offset, PVOID lpBuffer, ULONG cb,
                     PULONG lpcbBytesRead);

class SymbolReader
{
private:
    PVOID m_symbolReaderHandle;

    static std::string coreClrPath;
    static LoadSymbolsForModuleDelegate loadSymbolsForModuleDelegate;
    static DisposeDelegate disposeDelegate;
    static ResolveSequencePointDelegate resolveSequencePointDelegate;
    static GetLocalVariableName getLocalVariableNameDelegate;
    static GetLineByILOffsetDelegate getLineByILOffsetDelegate;
    static GetStepRangesFromIPDelegate getStepRangesFromIPDelegate;

    static HRESULT PrepareSymbolReader();

    // HRESULT GetNamedLocalVariable(
    //     ISymUnmanagedScope* pScope,
    //     ICorDebugILFrame* pILFrame,
    //     mdMethodDef methodToken,
    //     ULONG localIndex,
    //     WCHAR* paramName,
    //     ULONG paramNameLen,
    //     ICorDebugValue** ppValue);

    HRESULT LoadSymbolsForPortablePDB(
        WCHAR* pModuleName,
        BOOL isInMemory,
        BOOL isFileLayout,
        ULONG64 peAddress,
        ULONG64 peSize,
        ULONG64 inMemoryPdbAddress,
        ULONG64 inMemoryPdbSize);

public:
    SymbolReader()
    {
        m_symbolReaderHandle = 0;
    }

    ~SymbolReader()
    {
        if (m_symbolReaderHandle != 0)
        {
            disposeDelegate(m_symbolReaderHandle);
            m_symbolReaderHandle = 0;
        }
    }

    static void SetCoreCLRPath(const std::string &path) { coreClrPath = path; }

    HRESULT LoadSymbols(IMetaDataImport* pMD, ICorDebugModule* pModule);
    HRESULT GetLineByILOffset(mdMethodDef MethodToken, ULONG64 IlOffset, ULONG *pLinenum, WCHAR* pwszFileName, ULONG cchFileName);
    //HRESULT GetNamedLocalVariable(___in ICorDebugFrame * pFrame, ___in ULONG localIndex, __out_ecount(paramNameLen) WCHAR* paramName, ___in ULONG paramNameLen, ___out ICorDebugValue** ppValue);
    HRESULT ResolveSequencePoint(WCHAR* pFilename, ULONG32 lineNumber, TADDR mod, mdMethodDef* pToken, ULONG32* pIlOffset);
    HRESULT GetStepRangesFromIP(ULONG64 ip, mdMethodDef MethodToken, ULONG32 *ilStartOffset, ULONG32 *ilEndOffset);
};

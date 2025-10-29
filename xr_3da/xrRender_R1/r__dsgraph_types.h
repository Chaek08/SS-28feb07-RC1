#pragma once

#include	"fixedmap.h"

#ifndef USE_MEMORY_MONITOR
#	define USE_DOUG_LEA_ALLOCATOR_FOR_RENDER
#endif // USE_MEMORY_MONITOR

#ifdef USE_DOUG_LEA_ALLOCATOR_FOR_RENDER
#	include	"doug_lea_memory_allocator.h"

	extern "C" {
		void* dlmalloc(std::size_t);
		void  dlfree(void*);
	}

	template <class T>
	class doug_lea_alloc {
	public:
		typedef std::size_t   size_type;
		typedef std::ptrdiff_t difference_type;
		typedef T*            pointer;
		typedef const T*      const_pointer;
		typedef T&            reference;
		typedef const T&      const_reference;
		typedef T             value_type;

		template<class _Other>
		struct rebind { typedef doug_lea_alloc<_Other> other; };

	public:
		doug_lea_alloc() noexcept {}
		doug_lea_alloc(const doug_lea_alloc&) noexcept {}

		template<class U>
		doug_lea_alloc(const doug_lea_alloc<U>&) noexcept {}

		template<class U>
		doug_lea_alloc& operator=(const doug_lea_alloc<U>&) noexcept { return *this; }

		pointer allocate(size_type n, const void* = 0) {
			if (n == 0) n = 1;
			void* p = dlmalloc(n * sizeof(T));
			if (!p) throw std::bad_alloc();
			return static_cast<pointer>(p);
		}

		void deallocate(pointer p, size_type) noexcept {
			dlfree(static_cast<void*>(p));
		}

		void construct(pointer p, const T& v) {
			::new ((void*)p) T(v);
		}

		template <class... Args>
		void construct(pointer p, Args&&... args) {
			::new ((void*)p) T(std::forward<Args>(args)...);
		}

		void destroy(pointer p) noexcept {
			p->~T();
		}

		size_type max_size() const noexcept {
			return static_cast<size_type>(-1) / sizeof(T);
		}

		template<class U>
		bool operator==(const doug_lea_alloc<U>&) const noexcept { return true; }

		template<class U>
		bool operator!=(const doug_lea_alloc<U>&) const noexcept { return false; }
	};

	template<class _Ty,	class _Other>	inline	bool operator==(const doug_lea_alloc<_Ty>&, const doug_lea_alloc<_Other>&)		{	return (true);							}
	template<class _Ty, class _Other>	inline	bool operator!=(const doug_lea_alloc<_Ty>&, const doug_lea_alloc<_Other>&)		{	return (false);							}

	struct doug_lea_allocator {
		template <typename T>
		struct helper {
			typedef doug_lea_alloc<T>	result;
		};

		static	void	*alloc		(const u32 &n)	{	return dlmalloc((u32)n);	}
		template <typename T>
		static	void	dealloc		(T *&p)			{	dlfree(p);	p=0;			}
	};

#	define render_alloc				doug_lea_alloc
	typedef doug_lea_allocator		render_allocator;

#else // USE_DOUG_LEA_ALLOCATOR_FOR_RENDER
#	define render_alloc				xalloc
	typedef xr_allocator			render_allocator;
#endif // USE_DOUG_LEA_ALLOCATOR_FOR_RENDER

// #define	USE_RESOURCE_DEBUGGER

namespace	R_dsgraph
{
	// Elementary types
	struct _NormalItem	{
		float				ssa;
		IRender_Visual*		pVisual;
	};

	struct _MatrixItem	{
		float				ssa;
		IRenderable*		pObject;
		IRender_Visual*		pVisual;
		Fmatrix				Matrix;				// matrix (copy)
	};

	struct _MatrixItemS	: public _MatrixItem
	{
		ShaderElement*		se;
	};

	struct _LodItem		{
		float				ssa;
		IRender_Visual*		pVisual;
	};

#ifdef USE_RESOURCE_DEBUGGER
	typedef	ref_vs						vs_type;
	typedef	ref_ps						ps_type;
#else
	typedef	IDirect3DVertexShader9*		vs_type;
	typedef	IDirect3DPixelShader9*		ps_type;
#endif

	// NORMAL
	typedef xr_vector<_NormalItem,render_allocator::helper<_NormalItem>::result>			mapNormalDirect;
	struct	mapNormalItems		: public	mapNormalDirect										{	float	ssa;	};
	struct	mapNormalTextures	: public	FixedMAP<STextureList*,mapNormalItems,render_allocator>				{	float	ssa;	};
	struct	mapNormalStates		: public	FixedMAP<IDirect3DStateBlock9*,mapNormalTextures,render_allocator>	{	float	ssa;	};
	struct	mapNormalCS			: public	FixedMAP<R_constant_table*,mapNormalStates,render_allocator>			{	float	ssa;	};
	struct	mapNormalPS			: public	FixedMAP<ps_type, mapNormalCS,render_allocator>						{	float	ssa;	};
	struct	mapNormalVS			: public	FixedMAP<vs_type, mapNormalPS,render_allocator>						{	};
	typedef mapNormalVS			mapNormal_T;

	// MATRIX
	typedef xr_vector<_MatrixItem,render_allocator::helper<_MatrixItem>::result>	mapMatrixDirect;
	struct	mapMatrixItems		: public	mapMatrixDirect										{	float	ssa;	};
	struct	mapMatrixTextures	: public	FixedMAP<STextureList*,mapMatrixItems,render_allocator>				{	float	ssa;	};
	struct	mapMatrixStates		: public	FixedMAP<IDirect3DStateBlock9*,mapMatrixTextures,render_allocator>	{	float	ssa;	};
	struct	mapMatrixCS			: public	FixedMAP<R_constant_table*,mapMatrixStates,render_allocator>			{	float	ssa;	};
	struct	mapMatrixPS			: public	FixedMAP<ps_type, mapMatrixCS,render_allocator>						{	float	ssa;	};
	struct	mapMatrixVS			: public	FixedMAP<vs_type, mapMatrixPS,render_allocator>						{	};
	typedef mapMatrixVS			mapMatrix_T;

	// Top level
	typedef FixedMAP<float,_MatrixItemS,render_allocator>			mapSorted_T;
	typedef mapSorted_T::TNode						mapSorted_Node;

	typedef FixedMAP<float,_MatrixItemS,render_allocator>			mapHUD_T;
	typedef mapHUD_T::TNode							mapHUD_Node;

	typedef FixedMAP<float,_LodItem,render_allocator>				mapLOD_T;
	typedef mapLOD_T::TNode							mapLOD_Node;
};

/****************************************************************************
* VCGLib                                                            o o     *
* Visual and Computer Graphics Library                            o     o   *
*                                                                _   O  _   *
* Copyright(C) 2004                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *   
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/
/****************************************************************************
  History

$Log: not supported by cvs2svn $
Revision 1.2  2006/06/08 20:28:38  ganovelli
Corretto IsEnabledAttribute

Revision 1.1  2005/10/15 16:21:49  ganovelli
Working release (compilata solo su MSVC), vector_occ � migrato da component_opt

Revision 1.5  2005/07/07 13:33:52  ganovelli
some comment

Revision 1.4  2004/04/05 18:20:08  ganovelli
Aggiunto typename

Revision 1.3  2004/03/31 22:36:44  ganovelli
First Working Release (with this comment)

/****************************************************************************/

#ifndef __VCGLIB_TRACED_VECTOR__
#define __VCGLIB_TRACED_VECTOR__ 


#include <vcg/container/container_allocation_table.h>
#include <vcg/container/entries_allocation_table.h>

#include <assert.h>

namespace vcg {
	/*@{*/
/*!
 * This class represent a Traced Vector. A vector_occ is derived by a std::vector.
 * The characteristic of a vector_occ is that you can add (at run time) new attributes
 * to the container::value_type elements contained in the vector. (see the example..)
 * The position in memory of a traced vector is kept by the Container Allocation Table,
 * which is a (unique) list of vector_occ positions.
 */

template <class VALUE_TYPE>
class vector_occ: public std::vector<VALUE_TYPE>{
	typedef typename vector_occ<VALUE_TYPE> ThisType;

public:
	vector_occ():std::vector<VALUE_TYPE>(){id = ID(); ID()=ID()+1; reserve(1);}
	~vector_occ();
	
	VALUE_TYPE * Pointer2begin(){
		if(empty()) return (VALUE_TYPE *)id; else return &*std::vector<VALUE_TYPE>::begin();
	}
	std::list < CATBase<ThisType>* > attributes;
	// override di tutte le funzioni che possono spostare 
	// l'allocazione in memoria del container
	void push_back(const VALUE_TYPE & v);
	void pop_back();
	void resize(const unsigned int & size);
	void reserve(const unsigned int & size);

	/// this function enable the use of an optional attribute (see...)
	template <class ATTR_TYPE>
		void EnableAttribute(){
			CAT<ThisType,ATTR_TYPE> * cat = CAT<ThisType,ATTR_TYPE>::New();
			cat->Insert(*this);
			attributes.push_back(cat);
			}

	/// this function disable the use of an optional attribute (see...)
  /// Note: once an attribute is disabled, its data is lost (the memory freed)
	template <class ATTR_TYPE>
		bool IsEnabledAttribute(){
				std::list < CATBase<ThisType> * >::iterator ia; 
				for(ia = attributes.begin(); ia != attributes.end(); ++ia)
					if((*ia)->Id() == CAT<ThisType,ATTR_TYPE>::Id())
						return true;
				return false;
				}


	/// this function disable the use of an optional attribute (see...)
  /// Note: once an attribute is disabled, its data is lost (the memory freed)
	template <class ATTR_TYPE>
		void DisableAttribute(){
				std::list < CATBase<ThisType> * >::iterator ia; 
				for(ia = attributes.begin(); ia != attributes.end(); ++ia)
					if((*ia)->Id() == CAT<ThisType,ATTR_TYPE>::Id())
						{
							(*ia)->Remove(*this);
							//delete (*ia);
							attributes.erase(ia);
							break;
						}
				}

	/// this function create a new attribute of type ATTR_TYPE and return an handle to
  /// access the value of the attribute. Ex:
  /// vector_occ<float> tv;
  /// TempData<TVect,int> handle =  tv.NewTempData<int>();
  /// // now handle[&tv[123]] is the value of integer attribute associate with the position 123 on the vector
  /// // NOTE: it works also if you do some push_back, resize, pop_back, reserve that cause the relocation
  /// // of the vector_occ
	template <class ATTR_TYPE>
		TempData<ThisType,ATTR_TYPE> NewTempData(){
			typedef typename CATEntry<ThisType,EntryCATMulti<ThisType> >::EntryType EntryTypeMulti;
			CATEntry<ThisType,EntryTypeMulti>::Insert(*this);
			EntryTypeMulti	entry = CATEntry<ThisType,EntryTypeMulti >::GetEntry(Pointer2begin());
			entry.Data().push_back(new Wrap< ATTR_TYPE>);

			((Wrap<ATTR_TYPE>*)entry.Data().back())->reserve(capacity());
			((Wrap<ATTR_TYPE>*)entry.Data().back())->resize(size());

			return TempData<ThisType,ATTR_TYPE>((Wrap<ATTR_TYPE>*) entry.Data().back());
			}

	/// reciprocal of NewTempData
	template <class ATTR_TYPE>
		void DeleteTempData(TempData<ThisType,ATTR_TYPE> & td){
			typedef typename CATEntry<ThisType,EntryCATMulti<ThisType> >::EntryType EntryTypeMulti;
			CATEntry<ThisType,EntryTypeMulti >::RemoveIfEmpty(*this);
			EntryTypeMulti
				entry = CATEntry<ThisType,EntryCATMulti<ThisType> >::GetEntry(Pointer2begin);

			entry.Data().remove((Wrap<ATTR_TYPE>*)td.Item());
			delete ((Wrap<ATTR_TYPE>*)td.Item());
			}


private:	
	VALUE_TYPE * old_start;
	int id;
	static int & ID(){static int id; return id;} 
	void Update();
};

	/*@}*/

template <class VALUE_TYPE>
void vector_occ<VALUE_TYPE>::push_back(const VALUE_TYPE & v){
	std::vector<VALUE_TYPE>::push_back(v);
	Update();	
	std::list < CATBase<ThisType> * >::iterator ia; 
	for(ia = attributes.begin(); ia != attributes.end(); ++ia)
		(*ia)->AddDataElem(&(*(this->begin())),1);

}
template <class VALUE_TYPE>
void vector_occ<VALUE_TYPE>::pop_back(){
	std::vector<VALUE_TYPE>::pop_back();
	Update();
}

template <class VALUE_TYPE>
void vector_occ<VALUE_TYPE>::resize(const unsigned int & size){
	std::vector<VALUE_TYPE>::resize(size);
	Update();
	std::list < CATBase<ThisType> * >::iterator ia; 
	for(ia = attributes.begin(); ia != attributes.end(); ++ia)
		(*ia)->
	Resize(&(*(this->begin())),size);
}

template <class VALUE_TYPE>
void vector_occ<VALUE_TYPE>::reserve(const unsigned int & size){
	std::vector<VALUE_TYPE>::reserve(size);
	Update();
}

template <class VALUE_TYPE>
	void vector_occ<VALUE_TYPE>::
		Update(){
		std::list < CATBase<ThisType> * >::iterator ia; 
		if(Pointer2begin() != old_start)
			for(ia = attributes.begin(); ia != attributes.end(); ++ia)
				(*ia)->Resort(old_start,Pointer2begin());

		old_start = Pointer2begin();
	}



template <class VALUE_TYPE>
vector_occ<VALUE_TYPE>::~vector_occ(){
		std::list < CATBase<ThisType> * >::iterator ia; 
		for(ia = attributes.begin(); ia != attributes.end(); ++ia)
			{	
				(*ia)->Remove(*this);
				delete *ia;
			}
		}

}; // end namespace
#endif

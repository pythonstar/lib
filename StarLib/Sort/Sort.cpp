#include "stdafx.h"
#include "Sort.h"


/*------------------------------------------------------------------------
[7/24/2009 xiaolin]
说明:   	
------------------------------------------------------------------------*/
// 该函数模板使用冒泡法对集合元素进行排序，参数说明：
//     collection       集合对象，集合对象必须提供 [] 操作。
//     element          集合元素，该参数的作用仅仅是确定集合元素类型，
//                      参数的值没有用，建议取集合的第一个元素。集合
//                      元素必须提供复制、赋值和比较操作。
//     count            集合元素的数目
//     ascend           表明排序时使用升序(true)还是降序(false)
// 该函数模板支持C++数组以及MFC集合CStringArray、CArray。
template <typename COLLECTION_TYPE, typename ELEMENT_TYPE>
void Star::Sort::BubbleSort(COLLECTION_TYPE& collection, ELEMENT_TYPE element, int count, bool ascend)
{
	for (int i = 0; i < count-1; i++)
		for (int j = 0; j < count-1-i; j++)
			if (ascend)
			{
				// 升序
				if (collection[j] > collection[j+1])
				{
					ELEMENT_TYPE temp = collection[j];
					collection[j] = collection[j+1];
					collection[j+1] = temp;
				}
			}
			else
			{
				// 降序
				if (collection[j] < collection[j+1])
				{
					ELEMENT_TYPE temp = collection[j];
					collection[j] = collection[j+1];
					collection[j+1] = temp;
				}
			}
}



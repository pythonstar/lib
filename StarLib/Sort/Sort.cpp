#include "stdafx.h"
#include "Sort.h"


/*------------------------------------------------------------------------
[7/24/2009 xiaolin]
˵��:   	
------------------------------------------------------------------------*/
// �ú���ģ��ʹ��ð�ݷ��Լ���Ԫ�ؽ������򣬲���˵����
//     collection       ���϶��󣬼��϶�������ṩ [] ������
//     element          ����Ԫ�أ��ò��������ý�����ȷ������Ԫ�����ͣ�
//                      ������ֵû���ã�����ȡ���ϵĵ�һ��Ԫ�ء�����
//                      Ԫ�ر����ṩ���ơ���ֵ�ͱȽϲ�����
//     count            ����Ԫ�ص���Ŀ
//     ascend           ��������ʱʹ������(true)���ǽ���(false)
// �ú���ģ��֧��C++�����Լ�MFC����CStringArray��CArray��
template <typename COLLECTION_TYPE, typename ELEMENT_TYPE>
void Star::Sort::BubbleSort(COLLECTION_TYPE& collection, ELEMENT_TYPE element, int count, bool ascend)
{
	for (int i = 0; i < count-1; i++)
		for (int j = 0; j < count-1-i; j++)
			if (ascend)
			{
				// ����
				if (collection[j] > collection[j+1])
				{
					ELEMENT_TYPE temp = collection[j];
					collection[j] = collection[j+1];
					collection[j+1] = temp;
				}
			}
			else
			{
				// ����
				if (collection[j] < collection[j+1])
				{
					ELEMENT_TYPE temp = collection[j];
					collection[j] = collection[j+1];
					collection[j+1] = temp;
				}
			}
}



//***************************************************************************************
// IntervalAVLMap.h by Mohamed Ali(C) 2021 All Rights Reserved.
// Defines a template class for interval map.
// Interval Map Implemented as AVL tree
// --------------------------------------------------------------------------------------
// Key defined as (lo,hi) low is the BST's key
// This version of map stores both hi & lo values
// Assumptions 
// no two interval have the same left end point
// Key must be compare-able
// --------------------------------------------------------------------------------------
// max: is the maximum value at certain node
// height: is the node height
//***************************************************************************************

#ifndef INTERVAL_AVL_MAP_H
#define INTERVAL_AVL_MAP_H

#include <algorithm>
#include <memory>

template <class TKey>
struct Key
{
	Key() = default;
	Key(TKey lo, TKey hi)
	{
		(*this).lo = lo;
		(*this).hi = hi;
	}
	TKey lo, hi;
};

template <class TKey, class TValue>
struct Node
{
	Node() = default;

	Node(const Node<TKey, TValue>& node) :left(node.left.get()), right(node.right.get())
	{
		(*this).key = node.key;
		(*this).value = node.value;
		(*this).height = node.height;
	
		(*this).max = node.max;
	}

	Node(const Node<TKey,TValue>* node):left((*node).left.get()), right((*node).right.get())
	{
		(*this).key = (*node).key;
		(*this).value = (*node).value;
		(*this).height = (*node).height;
		(*this).max = (*node).max; 
	}
	Node(const Key<TKey>& key, const TValue& value)
	{
		(*this).key = key;
		(*this).value = value;
		height = 0;
		max = key.hi;
	}
	Key<TKey> key;
	TValue value;
	TKey max;
	int height;
	std::unique_ptr<Node<TKey, TValue>> left;
	std::unique_ptr<Node<TKey, TValue>> right;
};

template <class TKey, class TValue, class compare= std::less<TKey>>
class IntervalAVLMap 
{
public:
	IntervalAVLMap() = default;

	//simple function returns the first intersection it encounters
	//relace with your own implementation depends on your requirements
	const Node<TKey, TValue>* Find(TKey lo, TKey hi) const
	{
		if (m_pRoot.get() == nullptr) 
		{
			return nullptr;
		}
		return Find(lo, hi, m_pRoot);
	}
	const Node<TKey, TValue>* Find(TKey lo, TKey hi, const std::unique_ptr<Node<TKey,TValue>>& node) const
	{
		if ( node.get() == nullptr || hi>node->key.lo&&lo<node->key.hi)
		{
			return node.get();
		}
		else if (node->left.get() == nullptr || node->left->max < lo)
		{
			Find(lo, hi, node->right);
		}
		else
		{
			Find(lo, hi, node->left);
		}
	}
	Node<TKey, TValue>* Insert(TKey lo, TKey hi, TValue value)
	{
		if(hi<lo)
		{
			return nullptr;
		}
		Key<TKey> key= Key<TKey>(lo,hi);
		return Insert(key, value, m_pRoot).get();
	}

	std::unique_ptr<Node<TKey, TValue>>& Insert(Key<TKey>& key, TValue& value, std::unique_ptr<Node<TKey, TValue>>& node)
	{
		//if node empty add node
		if (node.get() == nullptr)
		{
			node= std::make_unique<Node<TKey, TValue>>(key, value);
			return node;
		}
		//if key is lesser than current add to left
		if (key.lo < (*node).key.lo)
		{
			(*node).left = std::move(Insert(key, value, (*node).left));
		}
		//if node is greater than current add to right
		else if (key.lo > (*node).key.lo)
		{
			(*node).right = std::move(Insert(key, value, (*node).right));
		}
		//if node exist update the value then return node
		else
		{
			(*node).value = value;
			return node;
		}
		
		Balance(node);
		(*node).height = CalcHeight(node.get());
		CalcMax(node.get());
		return node;
	}

	void Delete(TKey key)
	{
		Delete(key, m_pRoot);
	}
	void Delete(TKey key, std::unique_ptr<Node<TKey, TValue>>& node)
	{
		// node doesnt exist
		if (node.get() == nullptr)
		{
			return;
		}
		if ((*node).key.lo < key)
		{
			Delete(key, (*node).right);
		}
		else if (key < (*node).key.lo)
		{
			Delete(key, (*node).left);
		}
		else
		{
			// node with no childern
			if ((*node).height == 0)
			{
				node.reset();
				return;
			}
			//node with one child left
			else if ((*node).right.get() == nullptr)
			{
				std::unique_ptr<Node<TKey, TValue>> tmp = std::move((*node).left);
				node = std::move(tmp);
			}
			//node with one child right
			else if ((*node).left.get() == nullptr)
			{
				std::unique_ptr<Node<TKey, TValue>> tmp = std::move((*node).right);
				node = std::move(tmp);
			}
			//node with two childs 
			else
			{
				//copy key and delete the original key
				 auto min =(*FindMinimum((*node).right));
				 (*node).key = min.key;
				 (*node).value = min.value;
				
				Delete((*node).key.lo, (*node).right);
			}
		}
		
		Balance(node);
		(*node).height = CalcHeight(node.get());
	}

	
private:

	int GetMax(const Node<TKey, TValue>* node)
	{
		return (node == nullptr) ? -1 : (*node).max;
	}
	int GetHeight(const Node<TKey, TValue>* node)
	{
		return (node == nullptr) ? -1 : (*node).height;
	}
	Node<TKey, TValue>* const& operator[](TKey const& key) const
	{
		return Find(key);
	}
	
	void Balance(std::unique_ptr<Node<TKey, TValue>>& node)
	{
		int balance = GetBalance(node.get());
		//tree is right heavy if balance < -1
		if (balance < -1) 
		{
			//check if tree right subtree is left heavy 
			int subbalance = GetBalance((*node).right.get());
			if (subbalance >= 1)//sub left heavy
			{
				//double left rotation  RL
				RightRotation((*node).right);
			}
			LeftRotation(node);;
			(*node).height = CalcHeight(node.get());
			if ((*node).left.get() != nullptr)(*(*node).left).height = CalcHeight((*node).left.get());
			if ((*node).right.get() != nullptr)(*(*node).right).height = CalcHeight((*node).right.get());
		}
		//tree is left heavy if balance >1
		else if (balance > 1) 
		{
			//check if tree left subtree is right heavy
			int subbalance = GetBalance((*node).left.get());
			if (subbalance <= -1)
			{
				//double right rotation LR
				LeftRotation((*node).left);
			}
			RightRotation(node);
			(*node).height = CalcHeight(node.get());
			if ((*node).left.get() != nullptr)(*(*node).left).height = CalcHeight((*node).left.get());
			if ((*node).right.get() != nullptr)(*(*node).right).height = CalcHeight((*node).right.get());
		}


	}
	int CalcHeight(const Node<TKey, TValue>* node)
	{
		if (node == nullptr)
		{
			return -1;
		};
		int leftSubNode = ((*node).left.get() == nullptr) ? -1 : (*(*node).left).height;
		int rightSubNode = ((*node).right.get() == nullptr) ? -1 : (*(*node).right).height;
		return 1 + std::max(
			leftSubNode
			, rightSubNode);
	}
	int GetBalance(const Node<TKey, TValue>* node)
	{
		return GetHeight((*node).left.get()) - GetHeight((*node).right.get());
	}
	void RightRotation(std::unique_ptr<Node<TKey, TValue>>& node)
	{
		std::unique_ptr<Node<TKey, TValue>> temp;
		temp = std::move(node);
		node = std::move((*temp).left);
		(*temp).left = std::move((*node).right);
		CalcMax(temp.get());
		(*node).right = std::move(temp);
		CalcMax(node.get());

	}
	void LeftRotation(std::unique_ptr<Node<TKey, TValue>>& node)
	{
		std::unique_ptr<Node<TKey, TValue>> temp;
		temp = std::move(node);
		node = std::move((*temp).right);
		(*temp).right = std::move((*node).left);
		CalcMax(temp.get());
		(*node).left = std::move(temp);
		CalcMax(node.get());

	}
	void CalcMax(Node<TKey, TValue>* node)
	{
		if (node == nullptr)
		{
			return;
		}
		int leftMax = ((*node).left.get() == nullptr) ? -1 : (*(*node).left).max;
		int rightMax = ((*node).right.get() == nullptr) ? -1 : (*(*node).right).max;
		(*node).max = std::max({ (*node).key.hi,leftMax, rightMax });
	}
	const Node<TKey, TValue>* FindMinimum(const std::unique_ptr<Node<TKey, TValue>>& node)
	{   //node doesn't have left child
		if ((*node).left.get() == nullptr)
		{
			//node has no childern
			return node.get();
		}
		else
		{   // Find smaller value is exists
			FindMinimum((*node).left);
		}
	}
		
private:
	std::unique_ptr<Node<TKey, TValue>> m_pRoot;
};
template <class TKey, class TValue>
using IMap = IntervalAVLMap <TKey, TValue>;

#endif
#pragma once

#include "Engine\DebugSystem\Console.hpp"
#include "Engine\Utils\StringUtils.hpp"
#include "Engine\DebugSystem\ErrorWarningAssert.hpp"
#include <iostream>
#include <fstream>


//-------------------------------------------------------------------------------------------------
struct TreeNode
{
public:
	std::vector<TreeNode*> Children;
	std::vector<int> MetaData;
};


//-------------------------------------------------------------------------------------------------
void FillTree(TreeNode* CurrentNode, std::vector<int>& TreeInput, size_t& InputIndex)
{
	int ChildCount = TreeInput[InputIndex++];
	int MetaDataCount = TreeInput[InputIndex++];
	for (int ChildIndex = 0; ChildIndex < ChildCount; ++ChildIndex)
	{
		TreeNode* CurrentChild = new TreeNode();
		CurrentNode->Children.push_back(CurrentChild);
		FillTree(CurrentChild, TreeInput, InputIndex);
	}
	for (int MetaDataIndex = 0; MetaDataIndex < MetaDataCount; ++MetaDataIndex)
	{
		CurrentNode->MetaData.push_back(TreeInput[InputIndex++]);
	}
}


//-------------------------------------------------------------------------------------------------
int MetaDataSum(TreeNode* CurrentNode)
{
	int Sum = 0;
	if (CurrentNode->Children.size() == 0)
	{
		for (size_t Index = 0; Index < CurrentNode->MetaData.size(); ++Index)
		{
			Sum += CurrentNode->MetaData[Index];
		}
	}
	else
	{
		for (size_t Index = 0; Index < CurrentNode->MetaData.size(); ++Index)
		{
			int CurrentMetaData = CurrentNode->MetaData[Index] - 1;
			if (CurrentMetaData >= 0 && CurrentMetaData < (int)CurrentNode->Children.size())
			{
				Sum += MetaDataSum(CurrentNode->Children[CurrentMetaData]);
			}
		}
	}
	return Sum;
}


//-------------------------------------------------------------------------------------------------
void AdventOfCode()
{
	//Input
	std::string InputFilename = "Data/input.txt";
	std::ifstream InFile(InputFilename.c_str());
	ASSERT_OR_DIE(InFile, InputFilename + " is bad.");

	//Work
	std::string CurrentLine;
	getline(InFile, CurrentLine);
	std::vector<std::string> TreeInputString = SplitString(CurrentLine, ' ');
	std::vector<int> TreeInput;
	for (size_t Index = 0; Index < TreeInputString.size(); ++Index)
	{
		TreeInput.push_back(ParseInt(TreeInputString[Index]));
	}

	TreeNode* Root = new TreeNode();
	size_t Index = 0;
	FillTree(Root, TreeInput, Index);
	int Sum = MetaDataSum(Root);

	//Output
	g_ConsoleSystem->AddLog(Stringf("Solution: %d", Sum));
}
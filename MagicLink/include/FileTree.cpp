#include "FileTree.h"
#include "utils.h"
#include <iostream>

FileTree::FileTree(const std::wstring& name, NodeType type) :
	m_type(type),
	m_name(name)
{

}

FileTree* FileTree::addSubNode(const std::wstring & name, NodeType type)
{
	FileTree* newFileTree = new FileTree(name, type);
	m_subNodes[name] = newFileTree;

	if (m_path.length() > 0)
		newFileTree->m_path = m_path + L"\\" + m_name;
	else
		newFileTree->m_path = m_name;

	return newFileTree;
}

void FileTree::setName(const std::wstring& name)
{
	m_name = name;
}

void FileTree::setPath(const std::wstring& path)
{
	m_path = path;
}

void FileTree::setWriteTime(FILETIME time)
{
	m_lastWriteTime = time;
}

void FileTree::print(const std::wstring& indent) const
{
	std::wcout << indent << m_name << L"(" << m_path << L")" << std::endl;
	for (const auto elem : m_subNodes)
	{
		elem.second->print(indent + std::wstring(L"   "));
	}
}

void FileTree::showDiff(const FileTree& tree) const
{
	compare(this, &tree, nullptr);
}

std::wstring FileTree::getFullPath() const
{
	return m_path + L"\\" + m_name;
}

std::wstring FileTree::getName() const
{
	return m_name;
}

FileTreeDiff FileTree::getDiff(const FileTree & tree) const
{
	FileTreeDiff result;
	compare(this, &tree, &result);

	return result;
}

json FileTree::toJson() const
{
	json result;

	jsonBuilder(this, &result);

	return result;
}

void FileTree::compare(const FileTree* tree1, const FileTree* tree2, FileTreeDiff* result) const
{
	// Check if tree2 has all subNodes of tree1
	for (const auto elem : tree1->m_subNodes)
	{
		const std::wstring& name = elem.first;
		const FileTree* subNode = elem.second;

		// If yes -> check modifications
		if (tree2->hasSubNode(name))
		{
			const FileTree* subNode2 = tree2->m_subNodes.at(name);
			if (subNode->m_lastWriteTime == subNode2->m_lastWriteTime)
			{
				if (!result)
					std::wcout << L"[*] Modified '" << name << L"'" << std::endl;
			}

			compare(subNode, subNode2, result);
		}
		else // If no -> add it
		{
			if (result)
			{
				if (subNode->m_type == NodeTypeFILE)
				{
					result->filesToCreate.push_back(tree2->getFullPath() + L"\\" + name);
				}
				else
				{
					result->dirsToCreate.push_back(subNode->getFullPath());
				}
			}
			else
			{
				std::wcout << L"[+] New node '" << name << L"'" << std::endl;
			}
		}
	}

	for (const auto elem : tree2->m_subNodes)
	{
		const std::wstring& name = elem.first;
		const FileTree* subNode = elem.second;

		if (!tree1->hasSubNode(name))
		{
			if (result)
			{
				if (subNode->m_type == NodeTypeFILE)
				{
					result->filesToDelete.push_back(tree2->getFullPath() + L"\\" + name);
				}
				else
				{
					result->filesToDelete.push_back(subNode->getFullPath());
				}
			}
			else
				std::wcout << L"[-] Deleted node '" << name << L"'" << std::endl;
		}
	}
}

bool FileTree::hasSubNode(const std::wstring & name) const
{
	return (m_subNodes.find(name) != m_subNodes.end());
}

void FileTree::jsonBuilder(const FileTree* tree, json* result) const
{
	for (auto it : tree->m_subNodes)
	{
		std::string name(it.first.begin(), it.first.end());
		
		FileTree* subNode = it.second;
		
		if (subNode->m_type == NodeTypeFILE)
			(*result)[name]["type"] = "FILE";
		else
		{
			(*result)[name]["type"] = "DIR";
			jsonBuilder(subNode, &((*result)[name]["content"]));
		}
	}
}

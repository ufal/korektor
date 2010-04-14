
#include "utils.hpp"
#include "error_hierarchy.hpp"


const uint MINIMAL_ERROR_COUNT = 4;
const float error_count_normalization = 100;

class EstimateErrorModel
{
	hierarchy_nodeP hierarchy_root;
	map<u16string, uint32_t> context_map;

	bool estimate_error_model_rec(const hierarchy_nodeP &node)
	{
		if (node->children.empty())
		{
				u16string context;

				if (node->signature.substr(0, 5) == MyUtils::utf8_to_utf16("swap_"))
				{
					context += node->signature[6];
					context += node->signature[5];
				}
				else if (node->signature[0] == char16_t('s'))
				{
					context += node->signature[2];

				}
				else if (node->signature[0] == char16_t('i'))
				{
					context += node->signature[3];
					context += node->signature[2];
					context += node->signature[4];

				}
				else if (node->signature[0] == char16_t('d'))
				{
					context += node->signature[3];
				}
				else
				{
					cerr << "invalid signature: " << MyUtils::utf16_to_utf8(node->signature) << endl;
					return false;
				}

				if (context_map.find(context) == context_map.end())
				{
					//if (node->error_count > 0)
					//	cerr << "context not found: " << MyUtils::utf16_to_utf8(context) << endl;
					node->context_count = node->error_count;
					return false;
				}
				else
				{
					//if (node->error_count > 0)
					//	cerr << "---------------------context found" << endl;
				}

				node->context_count = context_map[context];

				if (node->error_count > MINIMAL_ERROR_COUNT)
				{
					node->error_prob = - log10((node->error_count * error_count_normalization) / context_map[context]);
					//out_nodes.push_back(node);
					node->is_output_node = true;
					cerr << "output leaf: " << MyUtils::utf16_to_utf8(node->signature) << endl;
					return true;
				}
				else
				{
					return false;
				}
		}
		else
		{
			FATAL_CONDITION(node->error_count == 0, "");
			uint context_count = 0;
			uint error_count = 0;

			for (auto it = node->children.begin(); it != node->children.end(); it++)
			{
				if (estimate_error_model_rec(*it) == false)
				{
					context_count += (*it)->context_count;
					error_count += (*it)->error_count;
				}
			}

			node->context_count = context_count;
			node->error_count = error_count;

			if (error_count > MINIMAL_ERROR_COUNT)
			{
				node->error_prob = -log10((error_count * error_count_normalization) / context_count);

				cerr << "node: " << MyUtils::utf16_to_utf8(node->signature) << ", context_count = " << node->context_count << ", error_count = " << node->error_count << endl;
				node->is_output_node = true;
				return true;
			}
			else
			{
				return false;
			}
		}
	}

public:
	EstimateErrorModel(hierarchy_nodeP _hierarchy_root, const map<u16string, uint32_t> &_context_map):
	  hierarchy_root(_hierarchy_root), context_map(_context_map) {}

	void Estimate()
	{
		estimate_error_model_rec(hierarchy_root);
	}
};

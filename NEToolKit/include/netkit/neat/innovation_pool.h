#pragma once

#include <vector>
#include <functional>
#include <optional>

#include "netkit/csv/serializer.h"
#include "netkit/csv/deserializer.h"
#include "netkit/network/network_primitive_types.h"
#include "neat_primitive_types.h"
#include "parameters.h"
#include "gene.h"
#include "innovation.h"
#include "population.h"

namespace netkit {
class innovation_pool {
  public:
	explicit innovation_pool(const parameters& params);
	innovation_pool(const innovation_pool& other) = default;
	innovation_pool(innovation_pool&& other) noexcept;
	innovation_pool& operator=(const innovation_pool& other) = default;
	innovation_pool& operator=(innovation_pool&& other) noexcept;

	innov_num_t next_innovation() { return m_next_innovation++; }
	neuron_id_t next_hidden_neuron_id() { return m_next_hidden_neuron_id++; }

	// just in case, const gene* is a pointer to a constant gene,
	// not a const pointer to a gene (that would be gene* const) nor
	// a const pointer to a const gene (const gene* const).
	std::optional<gene> find_gene(neuron_id_t from, neuron_id_t to);

	// like previous find_gene but search by innovation number.
	std::optional<gene> find_gene(innov_num_t innov_num);

	// please register a gene only if it isn't already registerd!
	void register_gene(gene new_gene);

	// similar to find_gene.
	std::optional<innovation> find_innovation(innov_type type, neuron_id_t from, neuron_id_t to);

	// similar to find_gene.
	std::optional<innovation> find_innovation(innov_num_t innov_num);

	// please register an innovation only if it isn't already registerd!
	void register_innovation(innovation new_innov);

	void clear();

  private:
	innov_num_t m_next_innovation;
	neuron_id_t m_next_hidden_neuron_id;
	std::vector<gene> m_all_genes;
	std::vector<innovation> m_all_innovations;

	template<typename func_t>
	inline std::optional<gene> helper_find_gene(func_t predicate) {
		auto it = std::find_if(
					  m_all_genes.begin(), m_all_genes.end(),
					  predicate
				  );

		if (it == m_all_genes.end()) {
			return {};
		}

		return *it;
	}

	template<typename func_t>
	inline std::optional<innovation> helper_find_innovation(func_t predicate) {
		auto it = std::find_if(
					  m_all_innovations.begin(), m_all_innovations.end(),
					  predicate
				  );

		if (it == m_all_innovations.end()) {
			return {};
		}

		return *it;
	}

	friend serializer& operator<<(serializer& ser, const innovation_pool& innov_pool);
	friend deserializer& operator>>(deserializer& des, innovation_pool& innov_pool);
};

serializer& operator<<(serializer& ser, const innovation_pool& innov_pool);
deserializer& operator>>(deserializer& des, innovation_pool& innov_pool);
}

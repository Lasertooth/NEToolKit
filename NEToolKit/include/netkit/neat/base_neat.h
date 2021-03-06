#pragma once

#include <optional>
#include <random>

#include "netkit/csv/serializer.h"
#include "netkit/csv/deserializer.h"
#include "parameters.h"
#include "innovation_pool.h"
#include "species.h"
#include "organism.h"

namespace netkit {
class base_population; // forward declaration

class base_neat {
  public:
	explicit base_neat(const parameters& params);
	base_neat(const base_neat& other);
	base_neat(base_neat&& other) noexcept;
	virtual ~base_neat() { delete m_best_genome_ever; }

	// init population with a default genome (all inputs connected to all outputs)
	void init();

	// init population with the given genome.
	void init(const genome& initial_genome);

	// you should have rated every organisms before calling this.
	void epoch();

	std::vector<species>& get_all_species() { return m_all_species; }

	std::optional<species*> find_appropriate_species_for(const genome& geno);

	const genome& get_current_best_genome() const;

	std::optional<genome> get_best_genome_ever() const;

	const std::vector<genome>& get_best_genomes_library() { return m_best_genomes_library; }

	std::optional<genome> get_random_genome_from_best_genome_library();

	// returns a pointer to the population to use (non-const version)
	virtual base_population* pop() = 0;

	// returns a pointer to the population to use (const version)
	virtual const base_population* pop() const = 0;

	// Call once per generation when population is rated.
	void update_best_genome_ever();

  protected:
	void helper_speciate_all_population();

	void helper_speciate_one_genome(genome_id_t geno_id);

	// update only if applicable.
	void helper_update_best_genomes_library_with(const genome& geno);

	void helper_serialize_base_neat(serializer& ser) const;

	void helper_deserialize_base_neat(deserializer& des);

	// overload this function for initialization
	virtual void impl_init(const genome& initial_genome) = 0;

	// overload this function for epoch implementation
	virtual void impl_epoch() = 0;

  public:
	parameters params;
	innovation_pool innov_pool;
	std::minstd_rand0 rand_engine;

  protected:
	std::vector<species> m_all_species;
	std::vector<genome> m_best_genomes_library;
	species_id_t m_next_species_id;
	genome* m_best_genome_ever;
	unsigned int m_age_of_best_genome_ever;
};
}

#include <algorithm> // std::sort, std::find
#include <limits> // std::numeric_limits

#include "netkit/neat/species.h"
#include "netkit/neat/base_neat.h"
#include "netkit/neat/base_population.h"

netkit::species::species(base_neat* neat_instance, base_population* population, species_id_t id,
						 const genome& representant)
	: m_members()
	, m_avg_fitness(0)
	, m_avg_adjusted_fitness(0)
	, m_best_fitness(0)
	, m_summed_fitnesses(0)
	, m_summed_adjusted_fitnesses(0)
	, m_best_fitness_ever(0)
	, m_id(id)
	, m_age(0)
	, m_age_of_last_improvement(0)
	, m_expected_offsprings(0)
	, m_sorted(false)
	, m_representant(new genome(representant))
	, m_neat(neat_instance)
	, m_population(population) {}

netkit::species::species(const species& other)
	: m_members(other.m_members)
	, m_avg_fitness(other.m_avg_fitness)
	, m_avg_adjusted_fitness(other.m_avg_adjusted_fitness)
	, m_best_fitness(other.m_best_fitness)
	, m_summed_fitnesses(other.m_summed_fitnesses)
	, m_summed_adjusted_fitnesses(other.m_summed_adjusted_fitnesses)
	, m_best_fitness_ever(other.m_best_fitness_ever)
	, m_id(other.m_id)
	, m_age(other.m_age)
	, m_age_of_last_improvement(other.m_age_of_last_improvement)
	, m_expected_offsprings(other.m_expected_offsprings)
	, m_sorted(other.m_sorted)
	, m_representant(new genome(*other.m_representant))
	, m_neat(other.m_neat)
	, m_population(other.m_population) {}

netkit::species::species(species&& other) noexcept
	: m_members(std::move(other.m_members))
	, m_avg_fitness(other.m_avg_fitness)
	, m_avg_adjusted_fitness(other.m_avg_adjusted_fitness)
	, m_best_fitness(other.m_best_fitness)
	, m_summed_fitnesses(other.m_summed_fitnesses)
	, m_summed_adjusted_fitnesses(other.m_summed_adjusted_fitnesses)
	, m_best_fitness_ever(other.m_best_fitness_ever)
	, m_id(other.m_id)
	, m_age(other.m_age)
	, m_age_of_last_improvement(other.m_age_of_last_improvement)
	, m_expected_offsprings(other.m_expected_offsprings)
	, m_sorted(other.m_sorted)
	, m_representant(other.m_representant) // steal the pointer HA!
	, m_neat(other.m_neat)
	, m_population(other.m_population) {
	other.m_representant = nullptr; // don't forget to invalidate the other's pointer.
}

netkit::species& netkit::species::operator=(const species& other) {
	if (&other == this) { // to avoid problems with dynamic allocation.
		return *this;
	}

	m_members = other.m_members;
	m_avg_fitness = other.m_avg_fitness;
	m_avg_adjusted_fitness = other.m_avg_adjusted_fitness;
	m_best_fitness = other.m_best_fitness;
	m_summed_fitnesses = other.m_summed_fitnesses;
	m_summed_adjusted_fitnesses = other.m_summed_adjusted_fitnesses;
	m_best_fitness_ever = other.m_best_fitness_ever;
	m_id = other.m_id;
	m_age = other.m_age;
	m_age_of_last_improvement = other.m_age_of_last_improvement;
	m_expected_offsprings = other.m_expected_offsprings;
	m_sorted = other.m_sorted;
	m_representant = new genome(*other.m_representant);
	m_neat = other.m_neat;
	m_population = other.m_population;

	return *this;
}

netkit::species& netkit::species::operator=(species&& other) noexcept {
	if (&other == this) {
		return *this;
	}

	m_members = std::move(other.m_members);
	m_avg_fitness = other.m_avg_fitness;
	m_avg_adjusted_fitness = other.m_avg_adjusted_fitness;
	m_best_fitness = other.m_best_fitness;
	m_summed_fitnesses = other.m_summed_fitnesses;
	m_summed_adjusted_fitnesses = other.m_summed_adjusted_fitnesses;
	m_best_fitness_ever = other.m_best_fitness_ever;
	m_id = other.m_id;
	m_age = other.m_age;
	m_age_of_last_improvement = other.m_age_of_last_improvement;
	m_expected_offsprings = other.m_expected_offsprings;
	m_sorted = other.m_sorted;
	m_representant = other.m_representant; // again pointer stealing (because we're in a move assignation)
	m_neat = other.m_neat;
	m_population = other.m_population;

	other.m_representant = nullptr; // very important!

	return *this;
}

netkit::genome_id_t netkit::species::get_champion() const {
	if (m_sorted) {
		return m_members.front();
	} else {
		genome_id_t best = m_members.front();
		double best_fitness_so_far = -1 * std::numeric_limits<double>::max();
		for (genome_id_t g : m_members) {
			if (m_population->get_genome(g).get_fitness() > best_fitness_so_far) {
				best = g;
				best_fitness_so_far = m_population->get_genome(g).get_fitness();
			}
		}
		return best;
	}
}

netkit::genome_id_t netkit::species::get_random_member() const {
	std::uniform_int_distribution<size_t> member_selector(0, m_members.size() - 1);
	size_t val = member_selector(m_neat->rand_engine);
	return m_members[val];
}

bool netkit::species::has(genome_id_t geno_id) const {
	return std::find(m_members.cbegin(), m_members.cend(), geno_id) != m_members.cend();
}

netkit::genome_id_t netkit::species::select_one_genitor() {
	if (!m_sorted) {
		sort_by_fitness();
	}

	if (m_best_fitness > 0) {
		std::uniform_real_distribution<double> distribution(0., 1.);
		double rnd_val = distribution(m_neat->rand_engine);
		for (genome_id_t g : m_members) {
			double genome_selection_prop = m_population->get_genome(g).get_fitness() / m_summed_fitnesses;
			if (rnd_val < genome_selection_prop) {
				return g;
			}
			rnd_val -= genome_selection_prop;
		}

		return m_members.back(); // just in case
	} else {
		return m_members.front();
	}
}

void netkit::species::sort_by_fitness() {
	if (!m_sorted) {
		std::sort(m_members.begin(), m_members.end(), [&](genome_id_t g1, genome_id_t g2) -> bool {
			return m_population->get_genome(g1).get_fitness() > m_population->get_genome(g2).get_fitness();
		});
		m_sorted = true;
	}
}

void netkit::species::update_stats() {
	m_avg_fitness = 0;
	m_avg_adjusted_fitness = 0;
	m_best_fitness = 0;
	m_summed_fitnesses = 0;
	m_summed_adjusted_fitnesses = 0;

	for (genome_id_t g : m_members) {
		m_summed_fitnesses += m_population->get_genome(g).get_fitness();
		m_summed_adjusted_fitnesses += m_population->get_genome(g).get_adjusted_fitness();

		if (m_population->get_genome(g).get_fitness() > m_best_fitness) {
			m_best_fitness = m_population->get_genome(g).get_fitness();
		}
	}
	m_avg_fitness = m_summed_fitnesses / static_cast<double>(m_members.size());
	m_avg_adjusted_fitness = m_summed_adjusted_fitnesses / static_cast<double>(m_members.size());

	if (m_best_fitness > m_best_fitness_ever) {
		m_best_fitness_ever = m_best_fitness;
		m_age_of_last_improvement = m_age;
	}
}

void netkit::species::init_for_next_gen(genome new_representant) {
	m_avg_fitness = 0;
	m_avg_adjusted_fitness = 0;
	m_best_fitness = 0;
	m_summed_fitnesses = 0;
	m_summed_adjusted_fitnesses = 0;
	++m_age;

	delete m_representant;
	m_representant = new genome(std::move(new_representant));

	m_members.clear();
	m_sorted = false;
}

void netkit::species::add_member(genome_id_t geno_id) {
	m_members.push_back(geno_id);
	m_sorted = false;
}

void netkit::species::remove_member(genome_id_t geno_id) {
	m_members.erase(std::remove(m_members.begin(), m_members.end(), geno_id), m_members.end());
}

void netkit::species::share_fitness() const {
	for (genome_id_t g : m_members) {
		m_population->get_genome(g).set_adjusted_fitness(
			m_population->get_genome(g).get_fitness() / static_cast<double>(m_members.size())
		);
	}
}

std::ostream& netkit::operator<<(std::ostream& os, const species& spec) {
	os << "<species: id = " << spec.m_id << ", age = " << spec.m_age
	   << ", age of last improvement = " << spec.m_age_of_last_improvement
	   << "\navg fitness = " << spec.m_avg_fitness
	   << ", best fitness = " << spec.m_best_fitness
	   << ", best fitness ever = " << spec.m_best_fitness_ever << std::endl;

	os << "members (" << spec.m_members.size() << ") are:" << std::endl;
	for (genome_id_t g : spec.m_members) {
		os << "-> " << spec.m_population->get_genome(g) << std::endl;
	}
	os << ">";

	return os;
}

netkit::serializer& netkit::operator<<(serializer& ser, const species& spec) {
	// The stats values shall can be computed back afterward, so we don't serialize them.

	// serialize important values
	ser.append(spec.m_id);
	ser.append(spec.m_age);
	ser.append(spec.m_age_of_last_improvement);
	ser.append(spec.m_sorted);
	ser.append(spec.m_best_fitness_ever);
	ser.new_line();

	// serialize the representant
	ser << *spec.m_representant;

	// serialize members
	ser.append(spec.m_members.size());
	ser.new_line();
	for (genome_id_t id : spec.m_members) {
		ser.append(id);
	}
	ser.new_line();

	return ser;
}

netkit::deserializer& netkit::operator>>(deserializer& des, species& spec) {
	// serialize important values
	des.get_next(spec.m_id);
	des.get_next(spec.m_age);
	des.get_next(spec.m_age_of_last_improvement);
	des.get_next(spec.m_sorted);
	des.get_next(spec.m_best_fitness_ever);

	// deserialize the representant
	genome repr(spec.m_neat);
	des >> repr;
	delete spec.m_representant; // important!
	spec.m_representant = new genome(std::move(repr));

	// deserialize members
	size_t number_of_members;
	des.get_next(number_of_members);
	spec.m_members.clear();
	spec.m_members.reserve(number_of_members);
	for (size_t i = 0; i < number_of_members; ++i) {
		genome_id_t id;
		des.get_next(id);
		spec.m_members.emplace_back(id);
	}

	return des;
}

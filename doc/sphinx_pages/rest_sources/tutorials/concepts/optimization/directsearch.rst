Direct Search Algorithms
========================

The Shark machine learning library offers classes and methods
implementing evolutionary single- and multi-objective optimization
algorithms. The component provides a multitude of different
evolutionary optimizers together with a rich sample of common
benchmark functions.

This section gives an overview of the underlying concepts of the
component and targets both the library user and a potential
contributor to Shark.

Conventions
-----------

In case of direct search algorithms, a minimization goal is
assumed. Single- and multi-objective optimization problems are treated
in uniformly. That is, fitness values are modelled as
vectors. In case of a single objective optimization problem, the size
of the vector equals 1.

Individuals & Populations
-------------------------

Most evolutionary algorithms, both single- and multi-objective, are
implemented in terms of the evolutionary loop. That is, in every
iteration of the algorithm, a set of individuals, a so-called
*population*, is evolved. In Shark, this concept is modelled in the
following way:

An individual consists of:

* multiple chromosomes,
* the search point associated with the individual as well as
* the fitness corresponding to the individual's search point.

Two different types of fitness values are supported, namely the
penalized and the unpenalized fitness. Whenever an individual's search
point violates one ore more constraints of the optimization problem at
hand, an evolutionary algorithm might choose to *encode* this
violation in terms of a penalized fitness value.

The class :doxy:`TypedIndividual` is templated w.r.t. the type of the search
point and w.r.t. the types of the chromosomes. For the latter ones,
the types are considered to be default- and copy-constructable. No
additional requirements on the types are assumed. To access the
chromosomes, a templated member function is provided::

 typedef IndividualBase< RealVector, int > Individual;
 Individual ind;
 ind.get< 0 >() = 5; // l-value semantic

Access to the search point is available by means of an overloaded
*-operator. Finally, unified access to the fitness value(s) assigned
to an individual is implemented in terms of a tagged function *fitness* with two different tags:

* PenalizedFitness()
* UnpenalizedFitness()

Populations, i.e., sets or multi-sets of individuals, are not modelled
explicitly. Instead, any container type can be used to represent
populations::

 typedef IndividualBase< RealVector, int > Individual;
 typedef std::set< Individual > Population;
 typedef std::vector< Individual> RandomAccessPopulation;

Evolutionary Operators - Selection, Variation, Evaluation
---------------------------------------------------------

In the Shark direct search component, typical ingredients of
evolutionary algorithms like mutation or crossover are modelled as
separate operators that do work on a single or a range of individuals. No
explicit interface is defined to abstract the functionality of
variation operates but we define loose concepts for every type of
operator.

Selection Operator
~~~~~~~~~~~~~~~~~~

A selection operator selects a certain individual from a set of
individuals. Thus, a selection operator selecting individuals
uniformly at random can be implemented in the following way::

  struct UniformSelection{
    template<typename Iterator>
    Iterator operator()( Iterator it, Iterator itE ) const {
      return( it + Rng::discrete( 0, std::distance( it, itE ) ) );
    }
  };

An iterator-based approach is favored in Shark. That is, the
underlying container are abstracted and no unneccesary copy of
individuals takes place.

Mutation Operator
~~~~~~~~~~~~~~~~~~

A mutation operator alters a single individual and might apply changes
to both the individual's search point as well as its chromosomes. In
general, const and non-const overloads should be available. For
example, a bitflip mutation can be implemented as follows::

  struct BitFlipMutation {
    // Non-const overload. Supplied individual is altered by the operator.
    template<typename Individual>
    void operator()( Individual & ind ) const {
      typedef typename Individual::SearchPointType PointType;
      for( PointType::iterator it = ind.begin(); it != ind.end(); ++it )
        if( shark::Rng::coinToss( 1./ind.size() ) )
      *it ^= *it;
    }

    // Const overload. Mutated individual is returned.
    template<typename Individual>
    Individual operator()( const Individual & ind ) const {
      Individual result;
      (*this)( result );
      return( result );
    }
  };

The bit-flip mutation operator can be applied to a range of
individuals by means of std::for_each::

  // Binary encoded individuals with no chromosomes.
  typedef TypedIndividual< std::vector< bool > > Individual;
  typedef std::list< Individual > Population;

  Population pop( 100, shark::make_individual( std::vector< bool >( 10 ) ) );
  std::for_each( pop.begin(), pop.end(), BitFlipMutation() );

Putting it all together, we can already define a very simple
evolutionary algorithm::

  #include <shark/Algorithms/DirectSearch/TypedIndividual.h>
  #include <shark/ObjectiveFunctions/Benchmarks/OneMax.h>

  #include <shark/Rng/GlobalRng.h>

  namespace shark {
    struct BitFlipMutation {
      template<typename Individual>
      void operator()( Individual & ind ) const {
        typedef typename Individual::SearchPointType PointType;
        PointType::iterator it = ind.begin() + Rng::discrete( 0, ind.size() - 1 );
        *it = ^*it;
      }

      template<typename Individual>
      Individual operator()( const Individual & ind ) const {
        Individual result;
        (*this)( result );
        return( result );
      }
    };
  }

  // Implements a (1+1)-GA
  int main( int argc, char ** argv ) {
    // Instantiate and configure the objective function.
    shark::OneMax oneMax;
    oneMax.setNoVariables( 10 );

    // Define types for individuals and populations.
    typedef shark::IndividualType< shark::BoolVector > Individual;


    // Generate, initialize and evaluate a parent individual.
    Individual parent( shark::BoolVector( 10, 0 ) );
    for( std::size_t i = 0; i < *(parent).size(); i++ )
      (*parent)( i ) = shark::Rng::coinToss( 0.5 );

    parent.fitness( shark::PenalizedFitness() ) =
    parent.fitness( shark::UnpenalizedFitness() ) =
    oneMax( *(*it) );

    Individual offspring( parent );

    shark::BitFlipMutation mutation;

    while( parent.fitness( shark::UnpenalizedFitness() ) > 0 ) {
      // Mating selection.
      offspring = parent;

      // Mutation.
      bfm( offspring );

      // Evaluation.
       offspring.fitness( shark::PenalizedFitness() ) =
       offspring.fitness( shark::UnpenalizedFitness() ) =
       oneMax( *(*it) );
      // Environmental selection.
      if(
        offspring.fitness( shark::UnpenalizedFitness() ) <=
        parent.fitness( shark::UnpenalizedFitness() ) )
      )
        std::swap( parent, offspring );
    }

    // Print the total number of fitness function evaluations to solve the problem.
    std::cout << "(1+1)-GA took: " << oneMax.evaluationCounter() << " to solve OneMax." << std::endl;

    return( EXIT_SUCCESS );
  }

Please see the example OnePlusOneGA for the complete source code and
refer to the more complex examples for further directions. Please see
the source code documentation to get a list of mutation operators
available in the Shark library.

Crossover Operators
~~~~~~~~~~~~~~~~~~~

Crossover or recombination refers to combining the characteristics of
two or more parent individuals for producing one or more offspring
individuals. In the Shark library, crossover operators are modelled as
function objects. Their characteristics like number of number of
parent and offspring individuals are modelled in terms of
RecombinationOperatorTraits. Thus, one point crossover operator can be
implemented in the following way::

  struct OnePointCrossover {
    template<typename Individual>
    Individual operator()(
      const Individual & mom,
      const Individual & dad,
      std::size_t point ) const {

      if( mom.size() != dad.size() )
        throw( SHARK_EXCEPTOIN( "Parents need to be of the same size." ) );

      Individual offspring( mom );
      std::copy( dad.begin() + point, dad.end(), offspring.begin() + point );

      return( offspring );
    }
  };

The operator takes two parent individuals *mom* and *dad* as input and
produces one offspring individual. We make known the input
and output arity of the operator by specializing
RecombinationOperatorTraits::

  template<> struct RecombinationOperatorTraits< OnePointCrossover > {
    static const std::size_t INPUT_ARITY = 2;
    static const std::size_t OUTPUT_ARITY = 1;
  };

Integrating Custom Optimizers & Objective Functions
---------------------------------------------------

Custom evolutionary optimizers and custom objective functions can be
integrated with the Shark library by inheriting
:doxy:`AbstractOptimizer` or :doxy:`AbstractObjectiveFunction`
respectively. Please see the abstract base classes
:doxy:`AbstractSingleObjectiveOptimizer`,
:doxy:`AbstractMultiObjectiveOptimizer`,
:doxy:`AbstractObjectiveFunction` and
the documentation in the file
:doxy:`AbstractObjectiveFunction.h`. In addition, Shark offers an
implementation of the factory pattern that can be populated both at
compile- and at runtime. To this end, developers can rely on the
following convenience macros to make optimizers and objective
functions known within Shark:

 * ANNOUNCE_SINGLE_OBJECTIVE_FUNCTION
 * ANNOUNCE_MULTI_OBJECTIVE_FUNCTION
 * ANNOUNCE_SINGLE_OBJECTIVE_OPTIMIZER
 * ANNOUNCE_MULTI_OBJECTIVE_OPTIMIZER


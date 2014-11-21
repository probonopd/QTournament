#ifndef ROUNDROBINGENERATOR_H
#define ROUNDROBINGENERATOR_H

#include <vector>
#include <tuple>


using namespace std;

namespace QTournament
{

class RoundRobinGenerator
{
public:
  RoundRobinGenerator();
  vector<tuple<int, int>> operator() (int numPlayers, int round);


private:
  int n(int r, int p);
};

}
#endif // ROUNDROBINGENERATOR_H

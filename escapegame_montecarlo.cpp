///////////////////////////////////////////////////////////////////////////////
///                                                                         ///
///  Monte Carlo simulation for the game "Escape: The Curse of the Temple"  ///
///  Hayden Jackson                                                         ///
///                                                                         ///
///////////////////////////////////////////////////////////////////////////////
#include <iostream> // cout, ostream
#include <fstream> // ofstream
#include <cstring> // strcmp
#include <cstdlib> // rand
#include <vector> // vector
#include <cmath> // sqrt

enum DieValue
{
	BLACK_MASK = 0,
	GOLD_MASK = 1,
	RED_TORCH = 2,
	BLUE_KEY = 3,
	GREEN_EXPLORER = 4,
	ANY_VALUE = 5		// this is not a side of a die, but a wildcard for any side of the die
};

DieValue RollDie()
{
	unsigned int result = rand() % 6;
	if(result == 5) result = 4; // GREEN_EXPLORER exists on two sides of the six-sided die
	return static_cast<DieValue>(result);
}

void PrintDieValues(const std::vector<DieValue> & dieValues, std::ostream & out)
{
	for(unsigned int die = 0; die < dieValues.size(); ++die)
	{
		switch(dieValues[die])
		{
			case BLACK_MASK:		out << "black mask"; break;
			case GOLD_MASK:			out << "gold mask"; break;
			case RED_TORCH:			out << "red torch"; break;
			case BLUE_KEY:			out << "blue key"; break;
			case GREEN_EXPLORER:	out << "green explorer"; break;
			case ANY_VALUE:			out << "[any roll]"; break;
		}

		if(die < dieValues.size() - 1)
			out << ", ";
	}
}


void EscapeGame_MonteCarlo(std::vector<DieValue> targetValues, unsigned int diceToRoll = 5, unsigned int trials = 10, std::ostream & out = std::cout, bool silentTrials = false, bool disableGoldMasks = false, bool keepBlackMasks = false)
{
	if(targetValues.size() > diceToRoll)
	{
		out << "Impossible to meet " << targetValues.size() << "-value target with only " << diceToRoll << (diceToRoll == 1 ? " die." : " dice.") << std::endl;
		return;
	}

	while(targetValues.size() < diceToRoll)
	{
		targetValues.push_back(ANY_VALUE);
	}

	out << "Target values: ";
	PrintDieValues(targetValues, out);
	out << std::endl;
	out << "Dice to roll: " << diceToRoll << std::endl;
	out << "Number of trials: " << trials << std::endl;

	std::vector<DieValue> currentValues;
	currentValues.resize(diceToRoll);

	unsigned int successes = 0;
	std::vector<unsigned int> rollsToSuccess;

	for(unsigned int trial = 0; trial < trials; ++trial)
	{
		if(!silentTrials)
			out << "~~~~~~~~~~ Trial #" << (trial + 1) << " ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;

		unsigned int lockedDice = 0; // dice that may not be rerolled; these are either black masks or dice that already match target values
		unsigned int unlockedMasks = 0;
		unsigned int blackMasks = 0;

		for(unsigned int die = 0; die < diceToRoll; ++die)
		{
			currentValues[die] = ANY_VALUE;
		}

		unsigned int roll = 0;
		while(lockedDice < diceToRoll)
		{
			if(!silentTrials)
				out << "Roll #" << (roll + 1) << " (" << (diceToRoll - lockedDice) << ((diceToRoll - lockedDice) > 1 ? " dice " : " die ") << "to roll, " << lockedDice << " locked): ";

			std::vector<DieValue> currentRolls;
			currentRolls.resize(diceToRoll - lockedDice);

			unsigned int goldMasks = 0;
			int newLockedDice = 0;

			// roll all dice that are not locked
			for(unsigned int die = 0; die < currentRolls.size(); ++die)
			{
				currentRolls[die] = RollDie();

				if(currentRolls[die] == BLACK_MASK)
				{
					++newLockedDice;
					++blackMasks;
				}
				else if(currentRolls[die] == GOLD_MASK)
				{
					++goldMasks;
				}
			}

			if(!silentTrials)
			{
				PrintDieValues(currentRolls, out);
				out << std::endl;
			}

			if(!disableGoldMasks)
			{
				if(goldMasks > 0)
				{
					unlockedMasks = blackMasks > (2 * goldMasks) ? (2 * goldMasks) : blackMasks;
					if(unlockedMasks > static_cast<unsigned int>(newLockedDice) && keepBlackMasks)
						unlockedMasks = newLockedDice;
					newLockedDice -= unlockedMasks;
					blackMasks -= unlockedMasks;
				}
			}

			bool success = true;
			unsigned int masksToUnlock = -newLockedDice < 0 ? 0 : -newLockedDice;

			// check the roll results against the target results
			for(unsigned int die = 0; die < diceToRoll; ++die)
			{
				if(currentValues[die] == targetValues[die])
				{
					if(currentValues[die] == BLACK_MASK && !keepBlackMasks && masksToUnlock > 0)
					{
						currentValues[die] = ANY_VALUE;
						--masksToUnlock;
						success = false;
					}

					continue;
				}

				success = false;

				for(unsigned int die2 = 0; die2 < currentRolls.size(); ++die2)
				{
					if(currentRolls[die2] == targetValues[die])
					{
						currentValues[die] = currentRolls[die2];

						if(currentRolls[die2] != BLACK_MASK) // black masks are marked as locked already
						{
							++newLockedDice;
						}
						else if(unlockedMasks - masksToUnlock > 0)
						{
							--unlockedMasks;
							if(keepBlackMasks)
							{			
								++newLockedDice;
								++blackMasks;
							}
							else
							{
								currentValues[die] = ANY_VALUE;
							}
						}

						for(unsigned int die3 = die2; die3 < currentRolls.size() - 1; ++ die3)
						{
							currentRolls[die3] = currentRolls[die3+1];
						}

						currentRolls.pop_back();
						
						break;
					}
				}
			}

			lockedDice += newLockedDice;
			++roll;

			if(success)
			{
				break;
			}
		}

		bool failure = false;
		for(unsigned int die = 0; die < diceToRoll; ++die)
		{
			if(currentValues[die] != targetValues[die])
			{
				failure = true;

				break;
			}
		}

		if(failure)
		{
			if(!silentTrials)
			{
				out << "### Target unsuccessfully met after " << roll << (roll > 1 ? " rolls." : " roll.") << " ###" << std::endl;

				out << "Final dice: ";
				PrintDieValues(currentValues, out);
				out << std::endl;
			}
		}
		else
		{
			++successes;
			rollsToSuccess.push_back(roll);
			if(!silentTrials)
			{
				out << "### Target successfully met after " << roll << (roll > 1 ? " rolls." : " roll.") << " ###" << std::endl;

				out << "Final dice: ";
				PrintDieValues(currentValues, out);
				out << std::endl;
			}
		}
	}

	out << "~~~~~~~~~~ Final Results ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
	out << "Successful trials: " << successes << " / " << trials << " (" << successes / float(trials) * 100 << "%)" << std::endl;

	float avgRollsToSuccess = 0;
	for(unsigned int successfulAttempt = 0; successfulAttempt < rollsToSuccess.size(); ++successfulAttempt)
	{
		avgRollsToSuccess += rollsToSuccess[successfulAttempt];
	}
	avgRollsToSuccess /= successes;

	float stdev = 0;
	for(unsigned int successfulAttempt = 0; successfulAttempt < rollsToSuccess.size(); ++successfulAttempt)
	{
		stdev += (rollsToSuccess[successfulAttempt] - avgRollsToSuccess) * (rollsToSuccess[successfulAttempt] - avgRollsToSuccess);
	}
	stdev = sqrt(stdev / successes);

	out << "Average number of rolls until success: " << avgRollsToSuccess << " (stdev: " << stdev << ")" << std::endl;
}

int main(int argc, char **argv)
{
	std::vector<DieValue> targetValues;
	unsigned int diceToRoll = 5;
	unsigned int trials = 10;
	std::ofstream outfile;
	bool silentTrials = false;
	bool disableGoldMasks = false;
	bool keepBlackMasks = false;

	if(argc < 2)
	{
		goto error;
	}

	for(int arg = 1; arg < argc; ++arg)
	{
		if(argv[arg][0] != '-')
		{
			goto error;
		}

		if(strcmp(argv[arg], "-bm") == 0)
		{
			if(++arg >= argc)
				goto error;

			unsigned int blackMasks = atoi(argv[arg]);
			for(unsigned int blackMask = 0; blackMask < blackMasks; ++blackMask)
			{
				targetValues.push_back(BLACK_MASK);
			}
		}
		else if(strcmp(argv[arg], "-gm") == 0)
		{
			if(++arg >= argc)
				goto error;

			unsigned int goldMasks = atoi(argv[arg]);
			for(unsigned int goldMask = 0; goldMask < goldMasks; ++goldMask)
			{
				targetValues.push_back(GOLD_MASK);
			}
		}
		else if(strcmp(argv[arg], "-rt") == 0)
		{
			if(++arg >= argc)
				goto error;
			
			unsigned int redTorches = atoi(argv[arg]);
			for(unsigned int redTorch = 0; redTorch < redTorches; ++redTorch)
			{
				targetValues.push_back(RED_TORCH);
			}
		}
		else if(strcmp(argv[arg], "-bk") == 0)
		{
			if(++arg >= argc)
				goto error;
			
			unsigned int blueKeys = atoi(argv[arg]);
			for(unsigned int blueKey = 0; blueKey < blueKeys; ++blueKey)
			{
				targetValues.push_back(BLUE_KEY);
			}
		}
		else if(strcmp(argv[arg], "-ge") == 0)
		{
			if(++arg >= argc)
				goto error;
			
			unsigned int greenExplorers = atoi(argv[arg]);
			for(unsigned int greenExplorer = 0; greenExplorer < greenExplorers; ++greenExplorer)
			{
				targetValues.push_back(GREEN_EXPLORER);
			}
		}
		else if(strcmp(argv[arg], "-dice") == 0)
		{
			if(++arg >= argc)
				goto error;

			diceToRoll = atoi(argv[arg]);
		}
		else if(strcmp(argv[arg], "-trials") == 0)
		{
			if(++arg >= argc)
				goto error;

			trials = atoi(argv[arg]);
		}
		else if(strcmp(argv[arg], "-outfile") == 0)
		{
			if(++arg >= argc)
				goto error;

			if(outfile.is_open())
				goto error;

			outfile.open(argv[arg]);

			if(!outfile.is_open())
				goto error;
		}
		else if(strcmp(argv[arg], "-disablegoldmasks") == 0)
		{
			disableGoldMasks = true;
		}
		else if(strcmp(argv[arg], "-silenttrials") == 0)
		{
			silentTrials = true;
		}
		else if(strcmp(argv[arg], "-keepblackmasks") == 0)
		{
			keepBlackMasks = true;
		}
		else
		{
			goto error;
		}
	}

	if(!outfile.is_open())
		std::cout << std::endl;

	EscapeGame_MonteCarlo(targetValues, diceToRoll, trials, outfile.is_open() ? outfile : std::cout, silentTrials, disableGoldMasks, keepBlackMasks);

	if(outfile.is_open())
		outfile.close();

	return 0;

	error:
		std::cout << "Invalid argument. Usage: \"" << argv[0] << " [-bm #] [-gm #] [-rt #] [-bk #] [-ge #] [-dice #] [-trials #] [-outfile log123.txt] [-silenttrials] [-disablegoldmasks] [-keepblackmasks]\"" << std::endl;
		return -1;
}

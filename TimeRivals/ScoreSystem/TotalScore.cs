using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TotalScore : MonoBehaviour
{
    private bool _firstUpdate = true;

    private GameObject[] _totalScoreboard; //Same as PlayerList but sorted by Rank

    public GameObject[] TotalScoreboard { get { return _totalScoreboard; } }

    private List<GameObject> _playerListRef;

    private void Awake()
    {
        _playerListRef = PlayerSetup.instance.PlayerList;
    }

    public void UpdateTotalScoreboard()
    {
        if (_firstUpdate)
        {
            _totalScoreboard = new GameObject[_playerListRef.Count]; //Set size of totalScoreboard same as Playerlist

            for (int i = 0; i < _playerListRef.Count; i++)
            {
                _totalScoreboard[i] = _playerListRef[i]; //Make an array of our playerlist so it's Identical, the only reason for doing this is so we can sort it based on Rank
            }

            _firstUpdate = false;
        }

        //Increment Total points for each player that they received during the level
        for (int i = 0; i < _totalScoreboard.Length; i++)
        {
            _totalScoreboard[i].GetComponent<PlayerController>().TotalPoints += _totalScoreboard[i].GetComponent<PlayerController>().Points;
        }

        //Update rankings
        SortTotalScoreboard();
    }

    public void SortTotalScoreboard()
    {
        for (int i = 0; i < _totalScoreboard.Length - 1; i++) //Sort so that highest points gets index 0(rank 1), lowest points gets index 3(rank 4)
        {
            for (int j = 0; j < _totalScoreboard.Length - i - 1; j++)
            {
                if (_totalScoreboard[j].GetComponent<PlayerController>().TotalPoints < _totalScoreboard[j + 1].GetComponent<PlayerController>().TotalPoints)
                {
                    GameObject temp = _totalScoreboard[j];
                    _totalScoreboard[j] = _totalScoreboard[j + 1];
                    _totalScoreboard[j + 1] = temp;
                }
            }
        }

        for (int i = 0; i < _totalScoreboard.Length; i++) //Assign the Totalrank for each player now that it's sorted
        {
            _totalScoreboard[i].GetComponent<PlayerController>().TotalRank = i;
        }

        foreach (GameObject player in _totalScoreboard)
        {
            Debug.Log(player.name + " is rank " + player.GetComponent<PlayerController>().TotalRank + " with " + player.GetComponent<PlayerController>().TotalPoints + " points.");
        }
    }
}
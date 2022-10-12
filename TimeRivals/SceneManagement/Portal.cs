using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Portal : MonoBehaviour
{
    private List<int> _playerIDs = new List<int>();
    public List<int> PlayerIDs { get { return _playerIDs; } }
    [Tooltip("PortalTypes: Start, Shop or Level")]
    [SerializeField] private string _portalType;

    private void OnTriggerEnter(Collider other)
    {
        if (other.gameObject.GetComponent<PlayerTriggers>()) //If it is a player
        {
            int otherID = other.gameObject.GetComponent<PlayerController>().PlayerID;

            if (!_playerIDs.Contains(otherID))
            {
                _playerIDs.Add(otherID);
            }

            if (_playerIDs.Count > 1 && _playerIDs.Count == PlayerSetup.instance.PlayerList.Count)
            {
                //Reset Variables for all players
                for (int i = 0; i < PlayerSetup.instance.PlayerList.Count; i++)
                {
                    PlayerSetup.instance.PlayerList[i].GetComponent<PlayerMovementBehaviour>().ResetAllVariables();
                }
                PlayerSetup.instance.ResetUseIndicator();

                switch (_portalType)
                {
                    case "Start":
                        LevelManager.Instance.StartGame();
                        break;
                    case "Shop":
                        LevelManager.Instance.LoadNextLevel();
                        break;
                    case "Level":
                        LevelManager.Instance.LoadShop();
                        break;
                }
                FMODUnity.RuntimeManager.PlayOneShot("event:/Master/SFX/Portal_Enter", transform.position);
            }
        }
    }

    private void OnTriggerExit(Collider other)
    {
        if (other.gameObject.GetComponent<PlayerTriggers>())
        {
            for (int i = 0; i < _playerIDs.Count; i++)
            {
                if (other.gameObject.GetComponent<PlayerController>().PlayerID == _playerIDs[i])
                {
                    _playerIDs.RemoveAt(i);
                }
            }
        }
    }
}

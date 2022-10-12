using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.InputSystem;
using UnityEngine.InputSystem.UI;
using UnityEngine.EventSystems;

public class PauseManager : MonoBehaviour
{
    public static PauseManager instance;

    [SerializeField] private GameObject _pauseMenu;
    [SerializeField] private GameObject _pauseMenuResumeButton;
    [SerializeField] private GameObject _pauseEventSystem;
    [SerializeField] private GameObject _pauseQuitButtonPC;

    List<GameObject> _playerList;

    private int _playerID = -1;
    public int PlayerID { get { return _playerID; } set { _playerID = value; } }

    private bool _paused;

    private void Awake()
    {
        instance = this;

#if UNITY_STANDALONE
        _pauseQuitButtonPC.SetActive(true);
#endif

    }

    public void DeterminePause(int playerID)
    {
        if (_paused && _playerID == playerID)
        {
            ResumeGame();
        }
        else if(_playerID == -1)
        {
            _playerID = playerID;
            PauseGame();
        }
    }

    public void PauseGame()
    {
        Time.timeScale = 0;
        _paused = true;

        _playerList = PlayerSetup.instance.PlayerList;

        foreach (GameObject player in _playerList)
        {
            if (player.GetComponent<PlayerController>().PlayerID == _playerID) //If this is the Player that paused
            {
                player.GetComponent<PlayerInput>().SwitchCurrentActionMap("UI");

                _pauseEventSystem.GetComponent<InputSystemUIInputModule>().actionsAsset = player.GetComponent<PlayerInput>().actions;
                continue;
            }

            player.GetComponent<PlayerInput>().DeactivateInput(); //Deactivate input for all other players
        }

        _pauseMenu.SetActive(true);

        GameObject.FindGameObjectWithTag("EventSystem").GetComponent<EventSystem>().SetSelectedGameObject(null);
        GameObject.FindGameObjectWithTag("EventSystem").GetComponent<EventSystem>().SetSelectedGameObject(_pauseMenuResumeButton);
    }

    public void ResumeGame()
    {
        _paused = false;

        _pauseMenu.SetActive(false);

        foreach (GameObject player in _playerList)
        {
            if (player.GetComponent<PlayerController>().PlayerID == _playerID) //If this is the Player that paused
            {
                player.GetComponent<PlayerInput>().SwitchCurrentActionMap("Player");
                continue;
            }

            player.GetComponent<PlayerInput>().ActivateInput(); //Active input for all players
        }

        _playerID = -1;
        Time.timeScale = 1;
    }

    public void ReturnToMainMenu()
    {
        Time.timeScale = 1;
        LevelManager.Instance.LoadMainMenu();
    }
}

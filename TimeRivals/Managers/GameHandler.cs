using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.InputSystem;

public class GameHandler : MonoBehaviour
{
    public static GameHandler instance = null;

    //
    [SerializeField] private float _respawnTimer;
    public float RespawnTimer { get { return _respawnTimer; } }

    //
    [SerializeField] private PlayerSpawnLocations _playerSpawnLocations;
    public PlayerSpawnLocations PlayerSpawnLocations { get { return _playerSpawnLocations; } }

    private int currSpawnIndex;

    private List<GameObject> _playerScoreboard = new List<GameObject>(); //will be sorted by Rank based on Points. Ex. [0] == first place, [3] == last place
    public List<GameObject> PlayerScoreboard { get { return _playerScoreboard; } }
    //

    [SerializeField] private GameObject _Canvas; //Needed to access the PlayerContainers in the UI
    private GameObject _playerContainerToUse;
    public GameObject PlayerContainerToUse { get { return _playerContainerToUse; } }


    //PlayerSetup.cs info
    private List<GameObject> _playerListRef;

    //Only used in scoreboard scene
    [SerializeField] private bool _inScoreboardScene;

    private void Awake()
    {
        if (instance == null)
        {
            instance = this;
        }
        else if (instance != null)
        {
            Destroy(gameObject);
        }
        /////////////////////////

        //Set References to PlayerSetup.cs
        _playerListRef = PlayerSetup.instance.PlayerList;

        //Reset Variables for all players
        for (int i = 0; i < _playerListRef.Count; i++)
        {
            _playerListRef[i].GetComponent<PlayerMovementBehaviour>().ResetAllVariables();
        }

        //Freeze all player input and reset their velocity;
        PlayerSetup.instance.FreezeAllPlayerInput();

        //Perform Initial Spawn for Players in current Scene
        for (int i = 0; i < _playerListRef.Count; i++)
        {
            _playerListRef[i].transform.position = _playerSpawnLocations.SpawnPoints[i];
            _playerListRef[i].GetComponent<HealthSystem>().MaxHealth = _playerListRef[i].GetComponent<PlayerController>().HealthBoost ? 150 : 100;
            _playerListRef[i].GetComponent<HealthSystem>().CurrHealth = _playerListRef[i].GetComponent<HealthSystem>().MaxHealth;
        }
        if (_inScoreboardScene)
        {
            Destroy(gameObject);
            return;
        }
        if (LevelManager.Instance.PlayMode == (int)PlayModes.UNSET || _Canvas == null) //if we are in the mainmenu or any scene without the ingame UI canvas
        {
            PlayerSetup.instance.UnFreezeAllPlayerInput();
            return;
        }
        
        //Choose correct UI playerContainer depending on Playmode
        EnablePlayerContainer();
        //Reset all player's temporary points from last round
        ResetPlayersPoints();

    }

    void Update()
    {
        int deadCount = 0;

        for (int i = 0; i < _playerListRef.Count; i++)
        {
            if (!_playerListRef[i].GetComponent<HealthSystem>().alive)
            {
                deadCount++;
            }
        }
        if (deadCount == _playerListRef.Count) //If all players are dead, spawn faster
        {
            for (int i = 0; i < _playerListRef.Count; i++)
            {
                if(!_playerListRef[i].GetComponent<HealthSystem>().hasInstaSpawn)
                {
                    _playerListRef[i].GetComponent<HealthSystem>().currRespawnTime++; //Speed up respawn time by 1 second if all players are currently dead
                    _playerListRef[i].GetComponent<HealthSystem>().hasInstaSpawn = true;
                }
            }
        }

        /////////////////////////////////////////////////////////////////////////////////////////

        for (int i = 0; i < _playerListRef.Count; i++)
        {
            if (_playerListRef[i].GetComponent<HealthSystem>().ShouldRespawn)
            {
                _playerListRef[i].GetComponent<HealthSystem>().respawnVFX.Play();
                _playerListRef[i].gameObject.transform.position = _playerSpawnLocations.SpawnPoints[currSpawnIndex];
                _playerListRef[i].GetComponent<HealthSystem>().ResetHealthVariables();
                _playerListRef[i].GetComponent<PlayerInput>().ActivateInput();
                currSpawnIndex++;
                
                if (currSpawnIndex == PlayerSpawnLocations.SpawnPoints.Length - 1)
                {
                    currSpawnIndex = 0;
                }
            }
        }

    }

    private void EnablePlayerContainer()
    {
        switch (LevelManager.Instance.PlayMode)
        {
            case (int)PlayModes.TWO_PLAYER:
                _playerContainerToUse = _Canvas.transform.GetChild(0).gameObject;
                break;

            case (int)PlayModes.THREE_PLAYER:
                _playerContainerToUse = _Canvas.transform.GetChild(1).gameObject;
                break;

            case (int)PlayModes.FOUR_PLAYER:
                _playerContainerToUse = _Canvas.transform.GetChild(2).gameObject;
                break;

            default:
                Debug.Log("Invalid TotalPlayerCount at EnablePlayerContainer");
                break;
        }

        _playerContainerToUse.SetActive(true);
        for (int i = 0; i < _playerListRef.Count; i++)
        {
            PlayerContainerPoints playerContainerPoints = _playerContainerToUse.transform.GetChild(i).gameObject.GetComponent<PlayerContainerPoints>();
            playerContainerPoints.Player = _playerListRef[i]; //Assign player to Correct UI Player Container
            _playerListRef[i].GetComponent<HealthSystem>().HealthBar = playerContainerPoints.Healthbar;//Assign Healthbar to Correct UI
            _playerListRef[i].GetComponent<HealthSystem>().DamageUIAnim = playerContainerPoints.gameObject.GetComponent<Animator>();//Assign Animator to display damage taken in UI

            PlayerController playerController = _playerListRef[i].GetComponent<PlayerController>(); //Get PlayController script from "i" player

            if(playerController.LightBoots)
                playerContainerPoints.powerUpContainer.transform.GetChild(0).gameObject.SetActive(true);
            else if (playerController.GripGloves)
                playerContainerPoints.powerUpContainer.transform.GetChild(1).gameObject.SetActive(true);
            else if (playerController.HealthBoost)
                playerContainerPoints.powerUpContainer.transform.GetChild(2).gameObject.SetActive(true);
            else if (playerController.HeavyBoots)
                playerContainerPoints.powerUpContainer.transform.GetChild(3).gameObject.SetActive(true);

            else //Else if player has no powerup, Disable all incase we had one from previous round
            {
               foreach (Transform child in playerContainerPoints.powerUpContainer.transform)
               {
                    child.gameObject.SetActive(false);
               }
            }
        }
    }

    private void ResetPlayersPoints()
    {
        for (int i = 0; i < _playerListRef.Count; i++)
        {
            _playerListRef[i].GetComponent<PlayerController>().Points = 0;
        }
    }

}

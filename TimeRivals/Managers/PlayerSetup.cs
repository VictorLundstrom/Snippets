using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.InputSystem;
using UnityEngine.InputSystem.UI;
using UnityEngine.UI;

public class PlayerSetup : MonoBehaviour
{
    public static PlayerSetup instance = null;
    public Color[] colors = new Color[4];
    private string _playerNamePrefix = "Player";
    private string _playerGrabColliderAppend = "GrabCollider";
    private string _playerGripperAppend = "Gripper";


    private List<GameObject> _playerList = new List<GameObject>();
    public List<GameObject> PlayerList { get { return _playerList; } }

    [SerializeField] private GameObject[] _playerPrefabs;
    public GameObject[] PlayerPrefabs { get { return _playerPrefabs; } }

    int _prefabIndex = 0;

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
    }

    public void OnPlayerJoined(PlayerInput playerInput)
    {
        if (LevelManager.Instance.PlayMode == (int)PlayModes.UNSET)
        {
            _playerList.Add(playerInput.gameObject);
            playerInput.gameObject.GetComponent<PlayerController>().PlayerID = playerInput.playerIndex; //Assign PlayerID
            playerInput.gameObject.transform.parent.gameObject.name = _playerNamePrefix + playerInput.playerIndex; //Assign Name to RootObject
            playerInput.gameObject.name = _playerNamePrefix + playerInput.playerIndex; //Same name as above but for the "Ragdoll" object itself
            playerInput.gameObject.GetComponent<HealthSystem>().PlayerRing.GetComponentInChildren<Image>().color = colors[playerInput.playerIndex]; // Change the player indicator ring's color based on with #player you are

            ParticleSystem.MainModule settings = playerInput.gameObject.GetComponent<HealthSystem>().respawnVFX.main;
            settings.startColor = new ParticleSystem.MinMaxGradient( colors[playerInput.playerIndex]);

            int playerLayer = LayerMask.NameToLayer(playerInput.gameObject.name); //Choose layer according to their playerID
            MoveToLayer(playerInput.gameObject.transform.parent, playerLayer); //Apply layer to all bodyparts in Player
            playerInput.gameObject.transform.GetChild(1).GetChild(2).gameObject.layer = LayerMask.NameToLayer(playerInput.gameObject.name + _playerGrabColliderAppend); //Set Unique layer for player's GrabCollider
            playerInput.gameObject.transform.parent.gameObject.GetComponent<ActiveRagdoll.GripModule>().PlayerLayer = playerInput.gameObject.layer; //Store layer for default player layer
            playerInput.gameObject.transform.parent.gameObject.GetComponent<ActiveRagdoll.GripModule>().GripLayer = LayerMask.NameToLayer(playerInput.gameObject.name + _playerGripperAppend); //Store layer for when Actively Gripping
            SwitchNextPrefab(); //Select a new prefab for next player that joins

            Debug.Log(playerInput.gameObject.name + " Joined the game");
        }

    }
    public void OnPlayerLeft(PlayerInput playerInput)
    {
        Debug.Log(playerInput.gameObject.name + " Left the game");
    }


    public void SwitchNextPrefab()
    {
        PlayerInputManager.instance.playerPrefab = PlayerPrefabs[_prefabIndex];
        _prefabIndex++;
    }

    public void FreezeAllPlayerInput()
    {
        foreach (GameObject player in _playerList)
        {
            player.GetComponent<PlayerInput>().DeactivateInput();
            ResetVelocity(player.transform.root);
            player.GetComponent<PlayerMovementBehaviour>().enabled = false;
        }
    }
    public void UnFreezeAllPlayerInput()
    {
        foreach (GameObject player in _playerList)
        {
            player.GetComponent<PlayerInput>().ActivateInput();
            player.GetComponent<PlayerMovementBehaviour>().enabled = true;
        }
    }
    public void SwitchActionMapForAll(string actionmap)
    {
        foreach (GameObject player in _playerList)
        {
            player.GetComponent<PlayerInput>().SwitchCurrentActionMap(actionmap);
        }
    }

    public void EnablePlayerEventSystemForAll()
    {
        foreach (GameObject player in _playerList)
        {
            GameObject playerEventSystem = player.transform.parent.GetChild(2).gameObject;
            playerEventSystem.SetActive(true);
        }
    }

    public void ResetPickupIndicator()
    {
        foreach (GameObject player in _playerList)
        {
            player.GetComponent<PlayerMovementBehaviour>().CanvasIndicator.transform.GetChild(0).gameObject.SetActive(false); //Child 0 needs to be the PickupIndicator
        }
    }

    public void ResetUseIndicator()
    {
        foreach (GameObject player in _playerList)
        {
            player.GetComponent<PlayerMovementBehaviour>().CanvasIndicator.transform.GetChild(2).gameObject.SetActive(false); //Child 2 needs to be the UseIndicator
        }
    }

    public void ResetElectrocution()
    {
        foreach (GameObject player in _playerList)
        {
            player.GetComponent<HealthSystem>().Electric.Stop();
            player.GetComponent<PlayerController>().playerAnimationBehavior.ToggleElectrocutionAnimation(false);
        }
    }

    public void InteruptAllPlayerStates()
    {
        foreach (GameObject player in _playerList)
        {
            player.GetComponent<PlayerMovementBehaviour>().InterruptPlayerState(PlayerStates.GRABBED);
        }
    }

    private void ResetVelocity(Transform root)
    {
        foreach (Transform child in root)
        {
            if (child.GetComponent<Rigidbody>())
            {
                child.GetComponent<Rigidbody>().velocity = Vector3.zero;
            }

            ResetVelocity(child);
        }
    }

    //Helper functions
    private void MoveToLayer(Transform root, int layer)
    {
        root.gameObject.layer = layer;
        foreach (Transform child in root)
            MoveToLayer(child, layer);
    }
}

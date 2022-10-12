using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.InputSystem;
using ActiveRagdoll;


public class PlayerController : MonoBehaviour
{
    [Header("Sub Behaviours")]
    public PlayerMovementBehaviour playerMovementBehaviour;
    public PlayerAnimationBehaviour playerAnimationBehavior;
    public Rumble rumble;
    public Transform playerRagdollTransform;

    [Header("Input Settings")]
    public float movementSmoothingSpeed = 1.0f;
    private Vector3 rawInputMovement;
    private Vector3 smoothInputMovement;
    private Vector3 smoothInputVelocity;
    private Vector3 rawInputRotation;

    [Header("Power Up's")]
    [SerializeField] private bool _lightBoots = false; //Increases the movement speed
    [SerializeField] private bool _heavyBoots = false; //Decreases the knockback force 25%
    [SerializeField] private bool _gripGloves = false; //Increases the number of times the gripped player needs to jump to get free
    [SerializeField] private bool _healthBoost = false; //Increases the number of hits required to die to 3 instead of 2.

    [Header("Misc")]
    [SerializeField] private int _playerID;
    private int _points;

    private int _totalPoints;
    public int TotalPoints { get { return _totalPoints; } set { _totalPoints = value; } }

    [SerializeField] private int _rank;
    private int _totalRank;
    public int TotalRank { get { return _totalRank; } set { _totalRank = value; } } 
    [SerializeField] private int _tokens = 0;

    // Setters & Getters /////////////////////////////////////////////////////////////////////////////////////////////////////
    public bool LightBoots { get { return _lightBoots; } set { _lightBoots = value; } }
    public bool HeavyBoots { get { return _heavyBoots; } set { _heavyBoots = value; } }
    public bool GripGloves { get { return _gripGloves; } set { _gripGloves = value; } }
    public bool HealthBoost { get { return _healthBoost; } set { _healthBoost = value; } }
    public int Tokens { get { return _tokens; } set { _tokens = value; } }
    public int PlayerID { get { return _playerID; } set { _playerID = value; } }
    public int Points { get { return _points; } set { _points = value; } }
    public int Rank { get { return _rank; } set { _rank = value; } }



    private void Update()
    {
        CalculateMovementInputSmoothing();
        UpdateAPlayerAnimationMovement();
        UpdatePlayerMovement();
    } //Update()


    public void OnMoveInput(InputAction.CallbackContext value)
    {
        Vector2 inputMovement = value.ReadValue<Vector2>();
        rawInputMovement = new Vector3(inputMovement.x, 0, inputMovement.y);
        playerMovementBehaviour.DustWalking.Play();
    }

    public void OnRotateInput(InputAction.CallbackContext value)
    {
        
        Vector2 inputRotation = value.ReadValue<Vector2>();
        rawInputRotation = new Vector3(inputRotation.x, 0, inputRotation.y);
    }

    public void OnJumpInput(InputAction.CallbackContext value)
    {
        if(value.started)
            playerMovementBehaviour.UpdateJumpData();
    }

    public void OnGrabInput(InputAction.CallbackContext value)
    {
        if (value.started)
        {
            playerMovementBehaviour.UpdateGrabData(true);
        }
        else if (value.canceled)
        {
            playerMovementBehaviour.UpdateGrabData(false);
        }
    }

    public void OnPickupInput(InputAction.CallbackContext context)
    {
        if (context.started || context.canceled)
        {
            playerMovementBehaviour.UpdatePickupData(context.ReadValueAsButton());
        }
    }

    public void OnUseInput(InputAction.CallbackContext context)
    {
        if (context.started || context.canceled)
        {
            playerMovementBehaviour.HandlePrinterButton(context.ReadValueAsButton());
        }
    }

    public void OnAttackInput(InputAction.CallbackContext context)
    {

        if (context.started)
        {
            playerMovementBehaviour.UpdateAttackData(true);
        }
    }
    public void OnPauseInput(InputAction.CallbackContext context)
    {
        if (context.performed)
        {
            PauseManager.instance.DeterminePause(_playerID);
        }
    }

    public void OnDestroyInput(InputAction.CallbackContext context)
    {
        if (context.started)
            playerMovementBehaviour.UpdateDestroyData();
    }
    public void ResetPowerups()
    {
        _lightBoots = false;
        _heavyBoots = false;
        _gripGloves = false;
        _healthBoost = false;
        _tokens = 0;
    }

    void UpdatePlayerMovement()
    {
        playerMovementBehaviour.UpdateMovementData(smoothInputMovement * (_lightBoots ? 1.3f : 1.1f));
        playerMovementBehaviour.UpdateRotationData(rawInputRotation);
    }

    void UpdateAPlayerAnimationMovement()
    {
        if (playerMovementBehaviour.PlayerState != PlayerStates.GRABBED && playerMovementBehaviour.PlayerState != PlayerStates.DEAD)
        {
            Vector3 dir = PlayerDirection(rawInputMovement);
            playerAnimationBehavior.UpdateDirectionAnimation(dir.x, dir.z);
        }
    }

    void CalculateMovementInputSmoothing()
    {
        smoothInputMovement = Vector3.SmoothDamp(smoothInputMovement, rawInputMovement,ref smoothInputVelocity, movementSmoothingSpeed);
    }

    Vector3 PlayerDirection(Vector3 rotationDirection)
    {
        var forward = playerRagdollTransform.forward;
        var left = -playerRagdollTransform.right;

        forward.y = 0.0f;
        left.y = 0.0f;

        return (forward * rotationDirection.z) + (left * rotationDirection.x);
    }
}

